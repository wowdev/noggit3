// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <noggit/TileWater.hpp>

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
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      delete chunks[z][x];
      chunks[z][x] = nullptr;
    }
  }
}

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

void TileWater::autoGen(float factor)
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

  int ofsW = lCurrentPosition + 0x8; //water Header pos

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header

  int headers_size = 256 * sizeof(MH2O_Header);
  // 8 empty bytes for the chunk header
  lADTFile.Extend(8 + headers_size);
  // set current pos after the mh2o headers
  lCurrentPosition = ofsW + headers_size;
  int header_pos = ofsW;


  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      chunks[z][x]->save(lADTFile, ofsW, header_pos, lCurrentPosition);
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

void TileWater::CropMiniChunk(int x, int z, MapChunk* chunkTerrain)
{
  chunks[z][x]->CropWater(chunkTerrain);
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
