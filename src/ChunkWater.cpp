#include "ChunkWater.h"

#include "MPQ.h"
#include "Liquid.h"
#include "Misc.h"
#include "MapChunk.h"

ChunkWater::ChunkWater(float pX, float pY)
  : x(pX)
  , y(pY)
{
  Liquids[0] = NULL;
}

ChunkWater::~ChunkWater()
{}

void ChunkWater::reloadRendering()
{
  if(!Header.nLayers) return;
  if(!Liquids[0]) Liquids[0] = new Liquid(Info[0].width, Info[0].height, Vec3D(x, Info[0].minHeight, y));

  MH2O_Tile lTile;
  lTile.mLiquidType = Info[0].LiquidType;
  lTile.mMaximum = Info[0].maxHeight;
  lTile.mMinimum = Info[0].minHeight;
  lTile.mFlags = Info[0].Flags;

  for( int x = 0; x < 9; ++x )
  {
    for( int y = 0; y < 9; ++y )
    {
      lTile.mHeightmap[x][y] = HeightData[0].mHeightValues[x][y];
      lTile.mDepth[x][y] = (HeightData[0].mTransparency[x][y] / 255.0f);
    }
  }

  memcpy(lTile.mRender, existsTable, 8*8);

  Liquids[0]->initFromMH2O(lTile);
}

void ChunkWater::fromFile(MPQFile &f, size_t basePos)
{
  f.read(&Header, sizeof(MH2O_Header));
  if(!Header.nLayers) return;

  for(int k=0; k < Header.nLayers; ++k)
  {
    if(k > 0) break; //Temporary only for 1 layer

    memset(existsTable[k], 0, 8*8);
    memset(InfoMask, 0, 8);

    //info
    f.seek(basePos + Header.ofsInformation);
    f.read(&Info[k], sizeof(MH2O_Information));

    //render
    if(Header.ofsRenderMask)
    {
      f.seek(basePos + Header.ofsRenderMask);
      f.read(&Render[k], sizeof(MH2O_Render));
    }
    else
    {
      memset(&Render[k].mask, 255, 8);
    }

    //mask
    if(Info[k].ofsInfoMask > 0 && Info[k].height > 0)
    {
      f.seek(Info[k].ofsInfoMask + basePos);
      size_t size(std::ceil(Info[k].height * Info[k].width / 8.0f));
      f.read(InfoMask, size);
    }

    int bitOffset = 0;

    for(int h = 0; h < Info[k].height; ++h)
    {
      for(int w = 0; w < Info[k].width; ++w)
      {
        bool render = true;

        if(Info[k].ofsInfoMask > 0)
        {
          render = (InfoMask[bitOffset / 8] >> (bitOffset % 8)) & 1;
          bitOffset++;
        }

        existsTable[k][h + Info[k].yOffset][w + Info[k].xOffset] = render;
      }
    }



    for(int h=0 ; h < 9; ++h)
    {
      for(int w=0; w < 9; ++w)
      {
        HeightData[k].mHeightValues[w][h] = Info[k].minHeight;
        HeightData[k].mTransparency[w][h] = 255;
      }
    }


    if(!Info[k].ofsHeightMap)
      return;

    f.seek(basePos + Info[k].ofsHeightMap);

    if(Info[k].Flags != 2)
    {
      for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
      {
        for(int h = Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
        {
          f.read(&HeightData[k].mHeightValues[w][h], sizeof(float));
        }
      }
    }

    for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
    {
      for(int h = Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
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

void ChunkWater::writeInfo(sExtendableArray &lADTFile, MH2O_Header *header, size_t basePos, int &lCurrentPosition)
{
  if(!hasData()) return;

  lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Information), reinterpret_cast<char*>(&Info[0])); //insert MH2O_Information
  header->ofsInformation = lCurrentPosition - basePos; //setting offset to this info at the header
  header->nLayers = 1;
  lCurrentPosition += sizeof(MH2O_Information);
}

void ChunkWater::writeData(MH2O_Header *header, sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition)
{
  if(!hasData()) return;

  MH2O_Information newinfo;

  newinfo.yOffset = 8;
  newinfo.xOffset = 8;
  newinfo.height = 0;
  newinfo.width = 0;
  newinfo.minHeight = 0;
  newinfo.maxHeight = 1000;
  newinfo.ofsHeightMap = 0;

  //render
  header->ofsRenderMask = lCurrentPosition - basePos;
  lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Render), reinterpret_cast<char*>(&Render[0]));
  lCurrentPosition += sizeof(MH2O_Render);

  for(int h = 0; h < 8; ++h)
  {
    for(int w = 0; w < 8; ++w)
    {
      if(!existsTable[0][h][w]) continue;

      if(w < newinfo.xOffset) newinfo.xOffset = w;
      if(h < newinfo.yOffset) newinfo.yOffset = h;

      if(w > newinfo.width) newinfo.width = w;
      if(h > newinfo.height) newinfo.height = h;
    }
  }

  newinfo.height -= newinfo.yOffset - 1;
  newinfo.width -= newinfo.xOffset - 1;

  uint8_t infoMask[8];
  int bitOffset = 0;

  memset(infoMask, 0, 8);

  for(int h = 0; h < newinfo.height; ++h)
  {
    for(int w = 0; w < newinfo.width; ++w)
    {
      infoMask[bitOffset / 8] |= (1 << (bitOffset % 8));
      bitOffset++;
    }
  }

  //mask
  lADTFile.Insert(lCurrentPosition, std::ceil(bitOffset / 8.0f), reinterpret_cast<char*>(infoMask));
  newinfo.ofsInfoMask = lCurrentPosition - basePos;
  lCurrentPosition += std::ceil(bitOffset / 8.0f);

  //HeighData & TransparencyData
  if(newinfo.LiquidType == 2) return;
  newinfo.ofsHeightMap = lCurrentPosition - basePos;

  for (int h = newinfo.yOffset; h < newinfo.yOffset + newinfo.height + 1; ++h)
  {
    for(int w = newinfo.xOffset; w < newinfo.xOffset + newinfo.width + 1; ++w)
    {
      if(HeightData[0].mHeightValues[h][w] < newinfo.minHeight) newinfo.minHeight = HeightData[0].mHeightValues[h][w];
      if(HeightData[0].mHeightValues[h][w] > newinfo.maxHeight) newinfo.maxHeight = HeightData[0].mHeightValues[h][w];


      lADTFile.Insert(lCurrentPosition, sizeof(float), reinterpret_cast<char*>(&HeightData[0].mHeightValues[h][w]));
      lCurrentPosition +=  sizeof(float);
    }
  }

  for (int h = newinfo.yOffset; h < newinfo.yOffset + newinfo.height + 1; ++h)
  {
    for(int w = newinfo.xOffset; w < newinfo.xOffset + newinfo.width + 1; ++w)
    {
      lADTFile.Insert(lCurrentPosition, sizeof(unsigned char), reinterpret_cast<char*>(&HeightData[0].mTransparency[h][w]));
      lCurrentPosition += sizeof(unsigned char);
    }
  }

  memcpy(lADTFile.GetPointer<char>(basePos + header->ofsInformation), &newinfo, sizeof(MH2O_Information));
}

void ChunkWater::autoGen(MapChunk *chunk, int factor)
{
  for(size_t y = 0; y < 9; ++y)
  {
    for(size_t x = 0; x < 9; ++x)
    {
      float terrainHeight(chunk->getHeight(y, x));
      float waterHeight(HeightData[0].mHeightValues[y][x]);

      int diff(factor * std::log(std::abs(waterHeight - terrainHeight) + 1.0f));
      diff = std::min(std::max(diff, 0), 255);

      HeightData[0].mTransparency[y][x] = diff;
    }
  }
  reloadRendering();
}

bool ChunkWater::hasLayer(size_t x, size_t y)
{
  if(!hasData()) return false;
  if(x > 8 || y > 8) return false;
  return existsTable[0][y][x];
}

void ChunkWater::addLayer()
{
  for(size_t y = 0; y < 8; ++y)
  {
    for(size_t x = 0; x < 8; ++x)
    {
      addLayer(x, y);
    }
  }
}

void ChunkWater::addLayer(size_t x, size_t y)
{
  if(hasLayer(x,y)) return;
  if(!hasData())
  {
    Header.nLayers = 1;
    reloadRendering();
  }
  existsTable[0][y][x] = true;
}

void ChunkWater::deleteLayer()
{
  for(size_t y = 0; y < 8; ++y)
  {
    for(size_t x = 0; x < 8; ++x)
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
  if(!hasLayer(x,y)) return;
  existsTable[0][y][x] = false;
}

void ChunkWater::setHeight(float height)
{
  for(size_t y = 0; y < 9; ++y)
  {
    for(size_t x = 0; x < 9; ++x)
    {
      HeightData[0].mHeightValues[y][x] = height;
    }
  }

  reloadRendering();
}

void ChunkWater::setHeight(size_t x, size_t y, float height)
{
  if(x > 8 || y > 8) return;
  if(!hasLayer(x,y)) return;

  HeightData[0].mHeightValues[y][x] = height;
  HeightData[0].mHeightValues[y+1][x] = height;
  HeightData[0].mHeightValues[y][x+1] = height;
  HeightData[0].mHeightValues[y+1][x+1] = height;

  reloadRendering();
}

float ChunkWater::getHeight()
{
  for(size_t y = 0; y < 9; ++y)
  {
    for(size_t x = 0; x < 9; ++x)
    {
      if(hasLayer(x,y)) return getHeight(x,y);
    }
  }
  return -1;
}

float ChunkWater::getHeight(size_t x, size_t y)
{
  if(!hasLayer(x,y)) return -1;
  return HeightData[0].mHeightValues[y][x];
}

int ChunkWater::getType()
{
  if(!hasData()) return 0;
  return Info[0].LiquidType;
}

void ChunkWater::setType(int type)
{
  if(!hasData()) return;
  Info[0].LiquidType = type;
  reloadRendering();
}

unsigned char ChunkWater::getTrans()
{
  for(size_t y = 0; y < 9; ++y)
  {
    for(size_t x = 0; x < 9; ++x)
    {
      if(hasLayer(x,y)) return getTrans(x,y);
    }
  }
  return 255;
}

unsigned char ChunkWater::getTrans(size_t x, size_t y)
{
  if(!hasLayer(x,y)) return 0;
  return HeightData[0].mTransparency[y][x];
}

void ChunkWater::setTrans(size_t x, size_t y, unsigned char trans)
{
  if(!hasLayer(x,y)) return;

  HeightData[0].mTransparency[y][x]  = trans;
  HeightData[0].mTransparency[y+1][x]  = trans;
  HeightData[0].mTransparency[y][x+1]  = trans;
  HeightData[0].mTransparency[y+1][x+1]  = trans;

  reloadRendering();
}

void ChunkWater::setTrans(unsigned char trans)
{
  for(size_t y = 0; y < 9; ++y)
  {
    for(size_t x = 0; x < 9; ++x)
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
  if(!hasData()) return;
  Liquids[0]->draw();
}
