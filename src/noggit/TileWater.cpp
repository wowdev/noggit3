#include "TileWater.hpp"

#include "Log.h"
#include "Misc.h"
#include "ChunkWater.hpp"

TileWater::TileWater(MapTile *pTile, float pXbase, float pZbase)
	: tile(pTile)
	, xbase(pXbase)
	, zbase(pZbase)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x] = new ChunkWater(xbase + CHUNKSIZE * x, zbase + CHUNKSIZE * z);
    }
  }
}

TileWater::~TileWater(void)
{}

void TileWater::readFromFile(MPQFile &theFile, size_t basePos)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			theFile.seek(basePos + (z * 16 + x) * sizeof(MH2O_Header));
			chunks[z][x]->fromFile(theFile, basePos);
		}
	}
	reload();
}

void TileWater::reload()
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->reloadRendering();
		}
	}
}

void TileWater::draw()
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->draw();
		}
	}
}

ChunkWater* TileWater::getChunk(int x, int z)
{
	return chunks[z][x];
}

void TileWater::autoGen(int factor)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->autoGen(tile->getChunk(x, z), factor);
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
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			headerOffsets[z][x] = lCurrentPosition;
			chunks[z][x]->writeHeader(lADTFile, lCurrentPosition);
		}
	}

	//writing MH2O_Information
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->writeInfo(lADTFile, ofsW, lCurrentPosition); //let chunk check if there is info!
		}
	}

	//writing other Info
	for (int z = 0; z < 16; ++z)
	{
		for (int x = 0; x < 16; ++x)
		{
			chunks[z][x]->writeData(headerOffsets[z][x], lADTFile, ofsW, lCurrentPosition);
		}
	}

	SetChunkHeader(lADTFile, ofsW - 8, 'MH2O', lCurrentPosition - ofsW);
	lCurrentPosition += 8;
}

bool TileWater::hasData(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return true;
      }
		}
	}

	return false;
}

void TileWater::deleteLayer(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->deleteLayer(layer);
		}
	}
}

void TileWater::deleteLayer(int x, int z, size_t layer)
{
	chunks[z][x]->deleteLayer(layer);
}


void TileWater::addLayer(float height, unsigned char trans, size_t layer)
{
	addLayer(layer);
	setHeight(height, layer);
	setTrans(trans, layer);
}

void TileWater::addLayer(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->addLayer(layer);
		}
	}
}

void TileWater::addLayer(int x, int z, size_t layer)
{
  chunks[z][x]->addLayer(layer);
}

void TileWater::addLayer(int x, int z, float height, unsigned char trans, size_t layer)
{
	chunks[z][x]->addLayer(layer);
	chunks[z][x]->setHeight(height, layer);
	chunks[z][x]->setTrans(trans, layer);
}

void TileWater::CropMiniChunk(int x, int z, MapChunk* chunkTerrain)
{
	chunks[z][x]->CropWater(chunkTerrain);
}

float TileWater::HaveWater(int x, int z)
{
	if (chunks[z][x]->hasData(0))
		return chunks[z][x]->getHeight(0);
	return 0;
}

void TileWater::setHeight(float height, size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->setHeight(height, layer);
		}
	}
}

void TileWater::setHeight(int x, int z, float height, size_t layer)
{
	chunks[z][x]->setHeight(height, layer);
}

float TileWater::getHeight(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return chunks[z][x]->getHeight(layer);
      }			
		}
	}
	return 0;
}

float TileWater::getHeightChunk(int x, int z, size_t layer)
{
  return chunks[z][x]->hasData(layer) ? chunks[z][x]->getHeight(layer) : 0;
}

void TileWater::setTrans(unsigned char opacity, size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->setTrans(opacity, layer);
		}
	}
}

unsigned char TileWater::getOpacity(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			if (chunks[z][x]->hasData(layer))
				return chunks[z][x]->getTrans(layer);
		}
	}
	return 255;
}

void TileWater::setType(int type, size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
			chunks[z][x]->setType(type, layer);
		}
	}
}

int TileWater::getType(size_t layer)
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      if (chunks[z][x]->hasData(layer))
      {
        return chunks[z][x]->getType(layer);
      }				
		}
	}
	return 0;
}
