#include "ChunkWater.h"

#include "MPQ.h"
#include "Liquid.h"
#include "Misc.h"
#include "MapChunk.h"

ChunkWater::ChunkWater(float pX, float pY)
	: x(pX)
	, y(pY)
{
  for (int i = 0; i < 5; ++i)
    Liquids[i] = nullptr;
}

ChunkWater::~ChunkWater()
{}

void ChunkWater::reloadRendering()
{
	if (!Header.nLayers) return;

  for (size_t k = 0; k < Header.nLayers; ++k)
  {
    if (!Liquids[k])
    {
      Liquids[k] = new Liquid(Info[k].width, Info[k].height, Vec3D(x, Info[k].minHeight, y));
    }      

    MH2O_Tile lTile;
    lTile.mLiquidType = Info[k].LiquidType;
    lTile.mMaximum = Info[k].maxHeight;
    lTile.mMinimum = Info[k].minHeight;
    lTile.mFlags = Info[k].Flags;

    for (int x = 0; x < 9; ++x)
    {
      for (int y = 0; y < 9; ++y)
      {
        lTile.mHeightmap[x][y] = HeightData[k].mHeightValues[x][y];
        lTile.mDepth[x][y] = (HeightData[k].mTransparency[x][y] / 255.0f);
      }
    }

    for (size_t i = 0; i < 8; ++i)
    {
      for (size_t j = 0; j < 8; ++j)
      {
        lTile.mRender[i][j] |= existsTable[k][i][j];
      }
    }

    Liquids[k]->initFromMH2O(lTile);
  }
}

void ChunkWater::fromFile(MPQFile &f, size_t basePos)
{
	f.read(&Header, sizeof(MH2O_Header));
	if (!Header.nLayers) return;

  size_t infoMaskPos, heightDataPos;

	for (int k = 0; k < (int)Header.nLayers; ++k)
	{
		memset(existsTable[k], 0, 8 * 8);
		memset(InfoMask, 0, 8);

		//info
		f.seek(basePos + Header.ofsInformation + sizeof(MH2O_Information)* k);
		f.read(&Info[k], sizeof(MH2O_Information));

		//render
		if (Header.ofsRenderMask)
		{
			f.seek(basePos + Header.ofsRenderMask + sizeof(MH2O_Render)* k);
			f.read(&Render[k], sizeof(MH2O_Render));
		}
		else
		{
			memset(&Render[k].mask, 255, 8);
		}

		//mask
		if (Info[k].ofsInfoMask > 0 && Info[k].height > 0)
		{
			f.seek(Info[k].ofsInfoMask + basePos);
			size_t size((size_t)(std::ceil(Info[k].height * Info[k].width / 8.0f)));
			f.read(InfoMask, size);
		}

		int bitOffset = 0;

		for (int h = 0; h < Info[k].height; ++h)
		{
			for (int w = 0; w < Info[k].width; ++w)
			{
				bool render = true;

				if (Info[k].ofsInfoMask > 0)
				{
					render = (InfoMask[bitOffset / 8] >> (bitOffset % 8)) & 1;
					bitOffset++;
				}

				existsTable[k][h + Info[k].yOffset][w + Info[k].xOffset] = render;
			}
		}

		for (int h = 0; h < 9; ++h)
		{
			for (int w = 0; w < 9; ++w)
			{
				HeightData[k].mHeightValues[w][h] = Info[k].minHeight;
				HeightData[k].mTransparency[w][h] = 255;
			}
		}

    if (!Info[k].ofsHeightMap)
    {
      continue;
    }			

    f.seek(basePos + Info[k].ofsHeightMap);

		if (Info[k].Flags != 2)
		{
			for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
			{
				for (int h = Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
				{
					f.read(&HeightData[k].mHeightValues[w][h], sizeof(float));
				}
			}
		}

		for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
		{
			for (int h = Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
			{
				f.read(&HeightData[k].mTransparency[w][h], sizeof(unsigned char));
			}
		}
	}
}

void ChunkWater::writeHeader(sExtendableArray &lADTFile, int &lCurrentPosition)
{
	lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Header), reinterpret_cast<char*>(&Header));
	lCurrentPosition += sizeof(MH2O_Header);
}

void ChunkWater::writeInfo(sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition)
{
	if (!hasData()) return;

  Header.ofsInformation = lCurrentPosition - basePos; //setting offset to this info at the header

  for (size_t i = 0; i < Header.nLayers; ++i)
  {
    lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Information), reinterpret_cast<char*>(&Info[i])); //insert MH2O_Information
    lCurrentPosition += sizeof(MH2O_Information);
  }	
}

void ChunkWater::writeData(size_t offHeader, sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition)
{
	if (!hasData()) return;

  Header.ofsRenderMask = lCurrentPosition - basePos;

  for (size_t i = 0; i < Header.nLayers; ++i)
  {
    Info[i].yOffset = 8;
    Info[i].xOffset = 8;
    Info[i].height = 0;
    Info[i].width = 0;
    Info[i].minHeight = HeightData[i].mHeightValues[0][0];
    Info[i].maxHeight = HeightData[i].mHeightValues[0][0];
    Info[i].ofsHeightMap = 0;

    //render

    lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Render), reinterpret_cast<char*>(&Render[i]));
    lCurrentPosition += sizeof(MH2O_Render);

    for (int h = 0; h < 8; ++h)
    {
      for (int w = 0; w < 8; ++w)
      {
        if (!existsTable[i][h][w]) continue;

        if (w < Info[i].xOffset) Info[i].xOffset = w;
        if (h < Info[i].yOffset) Info[i].yOffset = h;

        if (w > Info[i].width) Info[i].width = w;
        if (h > Info[i].height) Info[i].height = h;
      }
    }

    Info[i].height -= Info[i].yOffset - 1;
    Info[i].width -= Info[i].xOffset - 1;
  }

  for (size_t i = 0; i < Header.nLayers; ++i)
  {
    uint8_t infoMask[8];
    int bitOffset = 0;

    memset(infoMask, 0, 8);

    for (int h = 0; h < Info[i].height; ++h)
    {
      for (int w = 0; w < Info[i].width; ++w)
      {
        infoMask[bitOffset / 8] |= (1 << (bitOffset % 8));
        bitOffset++;
      }
    }

    //mask
    lADTFile.Insert(lCurrentPosition, (int)std::ceil(bitOffset / 8.0f), reinterpret_cast<char*>(infoMask));
    Info[i].ofsInfoMask = lCurrentPosition - basePos;
    lCurrentPosition += (int)std::ceil(bitOffset / 8.0f);
  }

  for (size_t i = 0; i < Header.nLayers; ++i)
  {
    //HeighData & TransparencyData
    Info[i].ofsHeightMap = lCurrentPosition - basePos;

    if (Info[i].Flags != 2)
    {
      for (int h = Info[i].yOffset; h < Info[i].yOffset + Info[i].height + 1; ++h)
      {
        for (int w = Info[i].xOffset; w < Info[i].xOffset + Info[i].width + 1; ++w)
        {
          Info[i].minHeight = std::min(HeightData[i].mHeightValues[h][w], Info[i].minHeight);
          Info[i].maxHeight = std::max(HeightData[i].mHeightValues[h][w], Info[i].maxHeight);

          lADTFile.Insert(lCurrentPosition, sizeof(float), reinterpret_cast<char*>(&HeightData[i].mHeightValues[h][w]));
          lCurrentPosition += sizeof(float);
        }
      }
    }

    for (int h = Info[i].yOffset; h < Info[i].yOffset + Info[i].height + 1; ++h)
    {
      for (int w = Info[i].xOffset; w < Info[i].xOffset + Info[i].width + 1; ++w)
      {
        lADTFile.Insert(lCurrentPosition, sizeof(unsigned char), reinterpret_cast<char*>(&HeightData[i].mTransparency[h][w]));
        lCurrentPosition += sizeof(unsigned char);
      }
    }
    memcpy(lADTFile.GetPointer<char>(basePos + Header.ofsInformation + sizeof(MH2O_Information)*i), &Info[i], sizeof(MH2O_Information));
  }
  memcpy(lADTFile.GetPointer<char>(offHeader), &Header, sizeof(MH2O_Header));
}

void ChunkWater::autoGen(MapChunk *chunk, int factor)
{
	for (size_t y = 0; y < 9; ++y)
	{
		for (size_t x = 0; x < 9; ++x)
		{
			float terrainHeight(chunk->getHeight(y, x));
			float waterHeight(HeightData[0].mHeightValues[y][x]);

			int diff(factor * (int)std::log(std::abs(waterHeight - terrainHeight) + 1.0f));
			diff = std::min(std::max(diff, 0), 255);

			HeightData[0].mTransparency[y][x] = diff;
		}
	}
	reloadRendering();
}

bool ChunkWater::hasLayer(size_t x, size_t y)
{
	if (!hasData()) return false;
	if (x > 8 || y > 8) return false;
	return existsTable[0][y][x];
}

void ChunkWater::addLayer()
{
	for (size_t y = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			addLayer(x, y);
		}
	}
}

void ChunkWater::CropWater(MapChunk* chunkTerrain)
{
	int k = 0;
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			if (hasLayer(j, i))
			{
				if (chunkTerrain->mVertices[k].y > getHeight(j, i))
					if (chunkTerrain->mVertices[k + 1].y > getHeight(j, i))
						if (chunkTerrain->mVertices[k + 17].y > getHeight(j, i))
							if (chunkTerrain->mVertices[k + 18].y > getHeight(j, i))
								deleteLayer(j, i);
			}
			++k;
		}
		k += 9;
	}
	reloadRendering();
	DelLayer();
}

void ChunkWater::DelLayer()
{
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			if (hasLayer(j, i))
				return;
	deleteLayer();
}

void ChunkWater::addLayer(size_t x, size_t y)
{
	if (hasLayer(x, y)) return;
	if (!hasData())
	{
		Header.nLayers = 1;
		reloadRendering();
	}
	existsTable[0][y][x] = true;
}

void ChunkWater::deleteLayer()
{
	for (size_t y = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			deleteLayer(x, y);
		}
	}
	Header.nLayers = 0;
	Info[0] = MH2O_Information();
	HeightData[0] = MH2O_HeightMask();
	Render[0] = MH2O_Render();
	delete Liquids[0];
	Liquids[0] = NULL;
}

void ChunkWater::deleteLayer(size_t x, size_t y)
{
	if (!hasLayer(x, y)) return;
	existsTable[0][y][x] = false;
}

void ChunkWater::setHeight(float height)
{
	for (size_t y = 0; y < 9; ++y)
	{
		for (size_t x = 0; x < 9; ++x)
		{
			HeightData[0].mHeightValues[y][x] = height;
		}
	}
	reloadRendering();
}

void ChunkWater::setHeight(size_t x, size_t y, float height)
{
	if (x > 8 || y > 8) return;
	if (!hasLayer(x, y)) return;

	HeightData[0].mHeightValues[y][x] = height;
	HeightData[0].mHeightValues[y + 1][x] = height;
	HeightData[0].mHeightValues[y][x + 1] = height;
	HeightData[0].mHeightValues[y + 1][x + 1] = height;

	reloadRendering();
}

float ChunkWater::getHeight()
{
	for (size_t y = 0; y < 9; ++y)
	{
		for (size_t x = 0; x < 9; ++x)
		{
			if (hasLayer(x, y)) return getHeight(x, y);
		}
	}
	return -1;
}

float ChunkWater::getHeight(size_t x, size_t y)
{
	if (!hasLayer(x, y)) return -1;
	return HeightData[0].mHeightValues[y][x];
}

int ChunkWater::getType()
{
	if (!hasData()) return 0;
	return Info[0].LiquidType;
}

void ChunkWater::setType(int type)
{
	if (!hasData()) return;
	Info[0].LiquidType = type;
	reloadRendering();
}

unsigned char ChunkWater::getTrans()
{
	for (size_t y = 0; y < 9; ++y)
	{
		for (size_t x = 0; x < 9; ++x)
		{
			if (hasLayer(x, y)) return getTrans(x, y);
		}
	}
	return 255;
}

unsigned char ChunkWater::getTrans(size_t x, size_t y)
{
	if (!hasLayer(x, y)) return 0;
	return HeightData[0].mTransparency[y][x];
}

void ChunkWater::setTrans(size_t x, size_t y, unsigned char trans)
{
	if (!hasLayer(x, y)) return;

	HeightData[0].mTransparency[y][x] = trans;
	HeightData[0].mTransparency[y + 1][x] = trans;
	HeightData[0].mTransparency[y][x + 1] = trans;
	HeightData[0].mTransparency[y + 1][x + 1] = trans;

	reloadRendering();
}

void ChunkWater::setTrans(unsigned char trans)
{
	for (size_t y = 0; y < 9; ++y)
	{
		for (size_t x = 0; x < 9; ++x)
		{
			HeightData[0].mTransparency[y][x] = trans;
		}
	}

	reloadRendering();
}

bool ChunkWater::hasData()
{
	return (Header.nLayers > 0);
}

void ChunkWater::draw()
{
  if (!hasData())
  {
    return;
  }    
  
  for (size_t i = 0; i < Header.nLayers; ++i)
  {
    Liquids[i]->draw();
  }	
}
