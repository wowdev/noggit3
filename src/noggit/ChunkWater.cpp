#include <noggit/ChunkWater.hpp>

#include <noggit/mpq/file.h>
#include <noggit/Liquid.h>
#include <noggit/MapChunk.h>

namespace noggit
{
  ChunkWater::ChunkWater(float pX, float pY)
  	: Liquids (nullptr)
    , x(pX)
  	, y(pY)
  {
  }

  void ChunkWater::reloadRendering()
  {
  	if (!Header.nLayers) return;
  	if (!Liquids) Liquids = new Liquid(Info.width, Info.height, ::math::vector_3d (x, Info.minHeight, y));

  	MH2O_Tile lTile;
  	lTile.mLiquidType = Info.LiquidType;
  	lTile.mMaximum = Info.maxHeight;
  	lTile.mMinimum = Info.minHeight;
  	lTile.mFlags = Info.Flags;

  	for (int x = 0; x < 9; ++x)
  	{
  		for (int y = 0; y < 9; ++y)
  		{
  			lTile.mHeightmap[x][y] = HeightData.mHeightValues[x][y];
  			lTile.mDepth[x][y] = (HeightData.mTransparency[x][y] / 255.0f);
  		}
  	}

  	memcpy(lTile.mRender, existsTable, 8 * 8);

  	Liquids->initFromMH2O(lTile);
  }

  void ChunkWater::fromFile(mpq::file &f, size_t basePos)
  {
  	f.read(&Header, sizeof(MH2O_Header));
  	if (!Header.nLayers) return;

    if (Header.nLayers > 1)
    {
      throw std::logic_error ("multiple MH2O layers nyi");
    }

  	for (int k = 0; k < (int)Header.nLayers; ++k)
  	{
  		memset(existsTable, 0, 8 * 8);
  		memset(InfoMask, 0, 8);

  		//info
  		f.seek(basePos + Header.ofsInformation);
  		f.read(&Info, sizeof(MH2O_Information));

  		//render
  		if (Header.ofsRenderMask)
  		{
  			f.seek(basePos + Header.ofsRenderMask);
  			f.read(&Render, sizeof(MH2O_Render));
  		}
  		else
  		{
  			memset(&Render.mask, 255, 8);
  		}

  		//mask
  		if (Info.ofsInfoMask > 0 && Info.height > 0)
  		{
  			f.seek(Info.ofsInfoMask + basePos);
  			size_t size((size_t)(std::ceil(Info.height * Info.width / 8.0f)));
  			f.read(InfoMask, size);
  		}

  		int bitOffset = 0;

  		for (int h = 0; h < Info.height; ++h)
  		{
  			for (int w = 0; w < Info.width; ++w)
  			{
  				bool render = true;

  				if (Info.ofsInfoMask > 0)
  				{
  					render = (InfoMask[bitOffset / 8] >> (bitOffset % 8)) & 1;
  					bitOffset++;
  				}

  				existsTable[h + Info.yOffset][w + Info.xOffset] = render;
  			}
  		}



  		for (int h = 0; h < 9; ++h)
  		{
  			for (int w = 0; w < 9; ++w)
  			{
  				HeightData.mHeightValues[w][h] = Info.minHeight;
  				HeightData.mTransparency[w][h] = 255;
  			}
  		}


  		if (!Info.ofsHeightMap)
  			return;

  		f.seek(basePos + Info.ofsHeightMap);

  		if (Info.Flags != 2)
  		{
  			for (int w = Info.yOffset; w < Info.yOffset + Info.height + 1; ++w)
  			{
  				for (int h = Info.xOffset; h < Info.xOffset + Info.width + 1; ++h)
  				{
  					f.read(&HeightData.mHeightValues[w][h], sizeof(float));
  				}
  			}
  		}

  		for (int w = Info.yOffset; w < Info.yOffset + Info.height + 1; ++w)
  		{
  			for (int h = Info.xOffset; h < Info.xOffset + Info.width + 1; ++h)
  			{
  				f.read(&HeightData.mTransparency[w][h], sizeof(unsigned char));
  			}
  		}
  	}
  }

  namespace
  {
    template<typename T>
      void insert (std::vector<char>& vector, T const& x, int& position)
    {
      vector.insert ( vector.begin() + position
                    , reinterpret_cast<char const*> (&x)
                    , reinterpret_cast<char const*> (&x) + sizeof (T)
                    );
      position += sizeof (T);
    }
    template<typename T>
      T* get_pointer (std::vector<char>& vector, size_t pPosition = 0)
    {
      return reinterpret_cast<T*> (&vector[pPosition]);
    }
  }

  void ChunkWater::writeHeader(std::vector<char> &adt_file, int &lCurrentPosition)
  {
    insert (adt_file, Header, lCurrentPosition);
  }

  void ChunkWater::writeInfo(std::vector<char> &adt_file, size_t basePos, int &lCurrentPosition)
  {
  	if (!hasData()) return;

    insert (adt_file, Info, lCurrentPosition);
  	Header.ofsInformation = lCurrentPosition - sizeof(MH2O_Information) - basePos; //setting offset to this info at the header
  	Header.nLayers = 1;
  }

  void ChunkWater::writeData(size_t offHeader, std::vector<char> &adt_file, size_t basePos, int &lCurrentPosition)
  {
  	if (!hasData()) return;

  	Info.yOffset = 8;
  	Info.xOffset = 8;
  	Info.height = 0;
  	Info.width = 0;
  	Info.minHeight = HeightData.mHeightValues[0][0];
  	Info.maxHeight = HeightData.mHeightValues[0][0];
  	Info.ofsHeightMap = 0;

  	//render
  	Header.ofsRenderMask = lCurrentPosition - basePos;
    insert (adt_file, Render, lCurrentPosition);

  	for (int h = 0; h < 8; ++h)
  	{
  		for (int w = 0; w < 8; ++w)
  		{
  			if (!existsTable[h][w]) continue;

  			if (w < Info.xOffset) Info.xOffset = w;
  			if (h < Info.yOffset) Info.yOffset = h;

  			if (w > Info.width) Info.width = w;
  			if (h > Info.height) Info.height = h;
  		}
  	}

  	Info.height -= Info.yOffset - 1;
  	Info.width -= Info.xOffset - 1;

  	uint8_t infoMask[8];
  	int bitOffset = 0;

  	memset(infoMask, 0, 8);

  	for (int h = 0; h < Info.height; ++h)
  	{
  		for (int w = 0; w < Info.width; ++w)
  		{
  			infoMask[bitOffset / 8] |= (1 << (bitOffset % 8));
  			bitOffset++;
  		}
  	}

  	//mask
    insert (adt_file, infoMask, lCurrentPosition);
  	Info.ofsInfoMask = lCurrentPosition - sizeof (infoMask) - basePos;

  	//HeighData & TransparencyData
  	Info.ofsHeightMap = lCurrentPosition - basePos;

  	for (int h = Info.yOffset; h < Info.yOffset + Info.height + 1; ++h)
  	{
  		for (int w = Info.xOffset; w < Info.xOffset + Info.width + 1; ++w)
  		{
  			if (HeightData.mHeightValues[h][w] < Info.minHeight) Info.minHeight = HeightData.mHeightValues[h][w];
  			if (HeightData.mHeightValues[h][w] > Info.maxHeight) Info.maxHeight = HeightData.mHeightValues[h][w];

        insert (adt_file, HeightData.mHeightValues[h][w], lCurrentPosition);
  		}
  	}

  	for (int h = Info.yOffset; h < Info.yOffset + Info.height + 1; ++h)
  	{
  		for (int w = Info.xOffset; w < Info.xOffset + Info.width + 1; ++w)
  		{
        insert (adt_file, HeightData.mTransparency[h][w], lCurrentPosition);
  		}
  	}

  	memcpy(get_pointer<char> (adt_file, offHeader), &Header, sizeof(MH2O_Header));
  	memcpy(get_pointer<char> (adt_file, basePos + Header.ofsInformation), &Info, sizeof(MH2O_Information));
  }

  void ChunkWater::autoGen(MapChunk *chunk, int factor)
  {
  	for (size_t y = 0; y < 9; ++y)
  	{
  		for (size_t x = 0; x < 9; ++x)
  		{
  			float terrainHeight(chunk->getHeight(y, x));
  			float waterHeight(HeightData.mHeightValues[y][x]);

  			int diff(factor * (int)std::log(std::abs(waterHeight - terrainHeight) + 1.0f));
  			diff = std::min(std::max(diff, 0), 255);

  			HeightData.mTransparency[y][x] = diff;
  		}
  	}
  	reloadRendering();
  }

  bool ChunkWater::hasLayer(size_t x, size_t y)
  {
  	if (!hasData()) return false;
  	if (x > 8 || y > 8) return false;
  	return existsTable[y][x];
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
  				if (chunkTerrain->mVertices[k].y() > getHeight(j, i))
  					if (chunkTerrain->mVertices[k + 1].y() > getHeight(j, i))
  						if (chunkTerrain->mVertices[k + 17].y() > getHeight(j, i))
  							if (chunkTerrain->mVertices[k + 18].y() > getHeight(j, i))
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
  	existsTable[y][x] = true;
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
  	Info = MH2O_Information();
  	HeightData = MH2O_HeightMask();
  	Render = MH2O_Render();
  	delete Liquids;
  	Liquids = nullptr;
  }

  void ChunkWater::deleteLayer(size_t x, size_t y)
  {
  	if (!hasLayer(x, y)) return;
  	existsTable[y][x] = false;
  }

  void ChunkWater::setHeight(float height)
  {
  	for (size_t y = 0; y < 9; ++y)
  	{
  		for (size_t x = 0; x < 9; ++x)
  		{
  			HeightData.mHeightValues[y][x] = height;
  		}
  	}
  	reloadRendering();
  }

  void ChunkWater::setHeight(size_t x, size_t y, float height)
  {
  	if (x > 8 || y > 8) return;
  	if (!hasLayer(x, y)) return;

  	HeightData.mHeightValues[y][x] = height;
  	HeightData.mHeightValues[y + 1][x] = height;
  	HeightData.mHeightValues[y][x + 1] = height;
  	HeightData.mHeightValues[y + 1][x + 1] = height;

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
  	return HeightData.mHeightValues[y][x];
  }

  int ChunkWater::getType()
  {
  	if (!hasData()) return 0;
  	return Info.LiquidType;
  }

  void ChunkWater::setType(int type)
  {
  	if (!hasData()) return;
  	Info.LiquidType = type;
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
  	return HeightData.mTransparency[y][x];
  }

  void ChunkWater::setTrans(size_t x, size_t y, unsigned char trans)
  {
  	if (!hasLayer(x, y)) return;

  	HeightData.mTransparency[y][x] = trans;
  	HeightData.mTransparency[y + 1][x] = trans;
  	HeightData.mTransparency[y][x + 1] = trans;
  	HeightData.mTransparency[y + 1][x + 1] = trans;

  	reloadRendering();
  }

  void ChunkWater::setTrans(unsigned char trans)
  {
  	for (size_t y = 0; y < 9; ++y)
  	{
  		for (size_t x = 0; x < 9; ++x)
  		{
  			HeightData.mTransparency[y][x] = trans;
  		}
  	}

  	reloadRendering();
  }

  bool ChunkWater::hasData()
  {
  	return (Header.nLayers > 0);
  }

  void ChunkWater::draw (Skies const* skies)
  {
  	if (!hasData()) return;
  	Liquids->draw (skies);
  }
}
