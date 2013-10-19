#include "TileWater.h"

#include "Log.h"
#include "Misc.h"
#include "ChunkWater.h"

TileWater::TileWater(MapTile *pTile, float pXbase, float pZbase)
  : tile(pTile)
  , xbase(pXbase)
  , zbase(pZbase)
{
  for(int i=0; i < 16; ++i)
    for(int j=0; j < 16; ++j)
        chunks[i][j] = new ChunkWater(xbase + CHUNKSIZE * j, zbase + CHUNKSIZE * i);
}

TileWater::~TileWater(void)
{}

void TileWater::readFromFile(MPQFile &theFile, size_t basePos)
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      theFile.seek(basePos + (i * 16 + j) * sizeof(MH2O_Header));
      chunks[i][j]->fromFile(theFile, basePos);
    }
  }
  reload();
}

void TileWater::reload()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->reloadRendering();
    }
  }
}

void TileWater::draw()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
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
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->autoGen(tile->getChunk(j, i), factor);
    }
  }
}

void TileWater::saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition)
{
  if(!hasData()) return;
  size_t ofsW = lCurrentPosition + 0x8; //water Header pos

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header
  lADTFile.Extend(8); // we need 8 empty bytes
  lCurrentPosition = ofsW;
  size_t headerOffsets[16][16];

  //writing MH2O_Header
  for(int i = 0; i < 16; ++i)
  {
    for(int j = 0; j < 16; ++j)
    {
      headerOffsets[i][j] = lCurrentPosition;
      chunks[i][j]->writeHeader(lADTFile, lCurrentPosition);
    }
  }

  //writing MH2O_Information
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      MH2O_Header *header(lADTFile.GetPointer<MH2O_Header>(headerOffsets[i][j]));

      chunks[i][j]->writeInfo(lADTFile, header, ofsW, lCurrentPosition); //let chunk check if there is info!
    }
  }

  //writing other Info
  for(int i = 0; i < 16; ++i)
  {
    for(int j = 0; j < 16; ++j)
    {
      MH2O_Header *header(lADTFile.GetPointer<MH2O_Header>(headerOffsets[i][j]));

      chunks[i][j]->writeData(header, lADTFile, ofsW, lCurrentPosition);
    }
  }

  SetChunkHeader(lADTFile, ofsW - 8, 'MH2O', lCurrentPosition - ofsW);
  lCurrentPosition += 8;
}

bool TileWater::hasData()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      if(chunks[i][j]->hasData()) return true;
    }
  }

  return false;
}

void TileWater::deleteLayer()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->deleteLayer();
    }
  }
}

void TileWater::addLayer(float height, unsigned char trans)
{
  addLayer();
  setHeight(height);
  setTrans(trans);
}

void TileWater::addLayer()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->addLayer();
    }
  }
}

void TileWater::setHeight(float height)
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->setHeight(height);
    }
  }
}

float TileWater::getHeight()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      if(chunks[i][j]->hasData())
        return chunks[i][j]->getHeight();
    }
  }
  return 0;
}

void TileWater::setTrans(unsigned char opacity)
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->setTrans(opacity);
    }
  }
}

unsigned char TileWater::getOpacity()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      if(chunks[i][j]->hasData())
        return chunks[i][j]->getTrans();
    }
  }
  return 255;
}

void TileWater::setType(int type)
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->setType(type);
    }
  }
}

int TileWater::getType()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      if(chunks[i][j]->hasData())
        return chunks[i][j]->getType();
    }
  }
  return 0;
}
