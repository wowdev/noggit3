#include "TileWater.h"

#include "Log.h"
#include "Misc.h"
#include "ChunkWater.h"

TileWater::TileWater(MapTile *pTile, float pXbase, float pZbase)
	: tile(pTile)
	, xbase(pXbase)
	, zbase(pZbase)
{
	for (int i = 0; i < 16; ++i)
		for (int j = 0; j < 16; ++j)
			chunks[i][j] = new ChunkWater(xbase + CHUNKSIZE * j, zbase + CHUNKSIZE * i);
}

TileWater::~TileWater(void)
{}

void TileWater::readFromFile(MPQFile &theFile, size_t basePos)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			theFile.seek(basePos + (i * 16 + j) * sizeof(MH2O_Header));
			chunks[i][j]->fromFile(theFile, basePos);
		}
	}
	reload();
}

void TileWater::reload()
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->reloadRendering();
		}
	}
}

void TileWater::draw()
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->draw();
		}
	}
}

ChunkWater* TileWater::getChunk(int x, int y)
{
	return chunks[x][y];
}

void TileWater::autoGen(int factor)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->autoGen(tile->getChunk(j, i), factor);
		}
	}
}

void TileWater::saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition)
{
  if (!hasData(0))
  {
    return;
  }

	size_t ofsW = lCurrentPosition + 0x8; //water Header pos

	lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header
	lADTFile.Extend(8); // we need 8 empty bytes
	lCurrentPosition = ofsW;
	size_t headerOffsets[16][16];

	//writing MH2O_Header
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			headerOffsets[i][j] = lCurrentPosition;
			chunks[i][j]->writeHeader(lADTFile, lCurrentPosition);
		}
	}

	//writing MH2O_Information
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->writeInfo(lADTFile, ofsW, lCurrentPosition); //let chunk check if there is info!
		}
	}

	//writing other Info
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->writeData(headerOffsets[i][j], lADTFile, ofsW, lCurrentPosition);
		}
	}

	SetChunkHeader(lADTFile, ofsW - 8, 'MH2O', lCurrentPosition - ofsW);
	lCurrentPosition += 8;
}

bool TileWater::hasData(size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
      if (chunks[i][j]->hasData(layer))
      {
        return true;
      }
		}
	}

	return false;
}

void TileWater::deleteLayer(size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->deleteLayer(layer);
		}
	}
}

void TileWater::deleteLayer(int i, int j, size_t layer)
{
	chunks[i][j]->deleteLayer(layer);
}


void TileWater::addLayer(float height, unsigned char trans, size_t layer)
{
	addLayer(layer);
	setHeight(height, layer);
	setTrans(trans, layer);
}

void TileWater::addLayer(size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->addLayer(layer);
		}
	}
}

void TileWater::addLayer(int i, int j, size_t layer)
{
  chunks[i][j]->addLayer(layer);
}

void TileWater::addLayer(int i, int j, float height, unsigned char trans, size_t layer)
{
	chunks[i][j]->addLayer(layer);
	chunks[i][j]->setHeight(height, layer);
	chunks[i][j]->setTrans(trans, layer);
}

void TileWater::CropMiniChunk(int i, int j, MapChunk* chunkTerrain)
{
	chunks[i][j]->CropWater(chunkTerrain);
}

float TileWater::HaveWater(int i, int j)
{
	if (chunks[j][i]->hasData(0))
		return chunks[j][i]->getHeight(0);
	return 0;
}

void TileWater::setHeight(float height, size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->setHeight(height, layer);
		}
	}
}

void TileWater::setHeight(int i, int j, float height, size_t layer)
{
	chunks[i][j]->setHeight(height, layer);
}

float TileWater::getHeight(size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
      if (chunks[i][j]->hasData(layer))
      {
        return chunks[i][j]->getHeight(layer);
      }			
		}
	}
	return 0;
}

float TileWater::getHeightChunk(int i, int j, size_t layer)
{
  return chunks[i][j]->hasData(layer) ? chunks[i][j]->getHeight(layer) : 0;
}

void TileWater::setTrans(unsigned char opacity, size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->setTrans(opacity, layer);
		}
	}
}

unsigned char TileWater::getOpacity(size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			if (chunks[i][j]->hasData(layer))
				return chunks[i][j]->getTrans(layer);
		}
	}
	return 255;
}

void TileWater::setType(int type, size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			chunks[i][j]->setType(type, layer);
		}
	}
}

int TileWater::getType(size_t layer)
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
      if (chunks[i][j]->hasData(layer))
      {
        return chunks[i][j]->getType(layer);
      }				
		}
	}
	return 0;
}
