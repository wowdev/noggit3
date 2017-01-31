// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/Environment.h>
#include <noggit/Liquid.h>
#include <noggit/MPQ.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>

ChunkWater::ChunkWater(float pX, float pY)
  : x(pX)
  , y(pY)
{
}

ChunkWater::~ChunkWater()
{
}



void ChunkWater::fromFile(MPQFile &f, size_t basePos)
{
  MH2O_Header header;
  f.read(&header, sizeof(MH2O_Header));
  if (!header.nLayers) return;

  //render
  if (header.ofsRenderMask)
  {
    f.seek(basePos + header.ofsRenderMask + sizeof(MH2O_Render));
    f.read(&Render, sizeof(MH2O_Render));
  }
  else
  {
    memset(&Render.mask, 255, 8);
  }

  for (int k = 0; k < (int)header.nLayers; ++k)
  {
    MH2O_Tile tile;
    MH2O_Information info;

    //info
    f.seek(basePos + header.ofsInformation + sizeof(MH2O_Information)* k);
    f.read(&info, sizeof(MH2O_Information));

    tile.mFlags = info.Flags;
    tile.mMinimum = info.minHeight;
    tile.mMaximum = info.maxHeight;
    tile.mLiquidType = info.LiquidType;

    //mask
    if (info.ofsInfoMask > 0 && info.height > 0)
    {
      uint64_t infoMask(0);
      f.seek(info.ofsInfoMask + basePos);
      f.read(&infoMask, 8);

      int bitOfs = 0;
      for (int y = 0; y < info.height; ++y)
      {
        for (int x = 0; x < info.width; ++x)
        {
          tile.mRender[y + info.yOffset][x + info.xOffset] = ((infoMask >> bitOfs) & 1);
          bitOfs++;
        }
      }
    }
    else
    {
      // no bitmap = all subchunks have water
      for (int y = 0; y < 8; ++y)
      {
        for (int x = 0; x < 8; ++x)
        {
          tile.mRender[y][x] = true;
        }
      }
    }   
    
    // set default value
    for (int h = 0; h < 9; ++h)
    {
      for (int w = 0; w < 9; ++w)
      {
        tile.mHeightmap[w][h] = info.minHeight;
        tile.mDepth[w][h] = 1.0f;
      }
    }

    // load existing heightMap
    if (info.ofsHeightMap)
    {      
      f.seek(info.ofsHeightMap + basePos);

      if (info.Flags != 2)
      {
        for (int w = info.yOffset; w < info.yOffset + info.height + 1; ++w)
        {
          for (int h = info.xOffset; h < info.xOffset + info.width + 1; ++h)
          {
            f.read(&tile.mHeightmap[w][h], sizeof(float));
          }
        }
      }

      for (int w = info.yOffset; w < info.yOffset + info.height + 1; ++w)
      {
        for (int h = info.xOffset; h < info.xOffset + info.width + 1; ++h)
        {
          unsigned char depth;
          f.read(&depth, sizeof(unsigned char));
          tile.mDepth[w][h] = depth / 255.0f;
        }
      }
    }

    _liquids.emplace_back(math::vector_3d(x, 0.0f, y), tile);
  }
}

void ChunkWater::reloadRendering()
{
  for (Liquid& layer : _liquids)
  {
    layer.updateRender();
  }
  
  /*if (!Header.nLayers) return;

  for (size_t k = 0; k < Header.nLayers; ++k)
  {
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

    Liquids[k] = new Liquid(Info[k].width, Info[k].height, math::vector_3d(x, Info[k].minHeight, y), lTile);
  }*/
}

void ChunkWater::writeHeader(sExtendableArray &lADTFile, int &lCurrentPosition)
{
  //lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Header), reinterpret_cast<char*>(&Header));
  //lCurrentPosition += sizeof(MH2O_Header);
}

void ChunkWater::writeInfo(sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition)
{
  /*
  if (!hasData(0))
  {
    return;
  }

  Header.ofsInformation = lCurrentPosition - basePos; //setting offset to this info at the header

  for (size_t i = 0; i < Header.nLayers; ++i)
  {
    lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Information), reinterpret_cast<char*>(&Info[i])); //insert MH2O_Information
    lCurrentPosition += sizeof(MH2O_Information);
  }*/
}

void ChunkWater::writeData(size_t offHeader, sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition)
{
  /*if (!hasData(0))
  {
    return;
  }

  Header.ofsRenderMask = lCurrentPosition - basePos;

  //render
  lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Render), reinterpret_cast<char*>(&Render));
  lCurrentPosition += sizeof(MH2O_Render);

  for (size_t i = 0; i < Header.nLayers; ++i)
  {
    Info[i].yOffset = 8;
    Info[i].xOffset = 8;
    Info[i].height = 0;
    Info[i].width = 0;
    Info[i].minHeight = HeightData[i].mHeightValues[0][0];
    Info[i].maxHeight = HeightData[i].mHeightValues[0][0];
    Info[i].ofsHeightMap = 0;

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
  memcpy(lADTFile.GetPointer<char>(offHeader), &Header, sizeof(MH2O_Header));*/
}

void ChunkWater::autoGen(MapChunk *chunk, int factor)
{
  /*
  for (size_t k = 0; k < Header.nLayers; ++k)
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
  }

  reloadRendering();
  */
}

bool ChunkWater::subchunkHasWater(size_t x, size_t y, size_t layer)
{
  /*if (layer >= Header.nLayers) return false;
  if (x > 8 || y > 8) return false;
  return existsTable[layer][y][x];*/
  return false;
}

void ChunkWater::addLayer(size_t layer)
{
  /*for (size_t y = 0; y < 8; ++y)
  {
    for (size_t x = 0; x < 8; ++x)
    {
      addLayer(x, y, layer);
    }
  }*/
}

void ChunkWater::CropWater(MapChunk* chunkTerrain)
{
  /*
  for (size_t k = 0; k < Header.nLayers; ++k)
  {
    for (int i = 0; i < 8; ++i)
    {
      for (int j = 0; j < 8; ++j)
      {
        if (subchunkHasWater(j, i, k))
        {
          float h = getHeight(j, i, k);
          if (chunkTerrain->mVertices[k].y > h &&
              chunkTerrain->mVertices[k + 1].y > h &&
              chunkTerrain->mVertices[k + 17].y > h &&
              chunkTerrain->mVertices[k + 18].y > h
            )
          {
            deleteLayer(j, i, k);
          }

        }
        ++k;
      }
      k += 9;
    }
    cleanLayer(k);
  }

  reloadRendering();
  */
}

void ChunkWater::cleanLayer(size_t layer)
{
  /*
  if (layer >= Header.nLayers)
  {
    return;
  }

  for (int i = 0; i < 8; ++i)
  {
    for (int j = 0; j < 8; ++j)
    {
      if (subchunkHasWater(j, i, layer))
      {
        return;
      }
    }
  }

  deleteLayer(layer);
  */
}

void ChunkWater::addLayer(size_t x, size_t y, size_t layer)
{
  /*
  if (subchunkHasWater(x, y, layer)) return;

  if (!hasData(layer))
  {
    Header.nLayers = layer+1;
    reloadRendering();
  }
  existsTable[layer][y][x] = true;
  */
}

void ChunkWater::deleteLayer(size_t layer)
{
  /*
  if (!hasData(layer))
  {
    return;
  }

  for (size_t y = 0; y < 8; ++y)
  {
    for (size_t x = 0; x < 8; ++x)
    {
      deleteLayer(x, y, layer);
    }
  }

  delete Liquids[layer];

  for (size_t k = layer; k < Header.nLayers; ++k)
  {
    Liquids[k] = Liquids[k + 1];
    Info[k] = Info[k + 1];
    HeightData[k] = HeightData[k + 1];
    memcpy(existsTable[k], existsTable[k + 1], 64);
  }

  Header.nLayers--;
  Liquids[Header.nLayers] = nullptr;

  for (size_t i = 0; i < 8; ++i)
  {
    for (size_t j = 0; j < 8; j++)
    {
      existsTable[Header.nLayers][i][j] = false;
    }
  }

  if (Header.nLayers == 0)
  {
    Info[0] = MH2O_Information();
    HeightData[0] = MH2O_HeightMask();
    Render = MH2O_Render();
  }
  */
}

void ChunkWater::deleteLayer(size_t x, size_t y, size_t layer)
{
  /*
  if (subchunkHasWater(x, y, layer))
  {
    existsTable[layer][y][x] = false;
  }
  */
}

void ChunkWater::setHeight(float height, size_t layer)
{
  /*
  for (size_t y = 0; y < 9; ++y)
  {
    for (size_t x = 0; x < 9; ++x)
    {
      HeightData[layer].mHeightValues[y][x] = height;
    }
  }
  reloadRendering();
  */
}

void ChunkWater::setHeight(size_t x, size_t y, float height, size_t layer)
{
  /*
  if (x > 8 || y > 8 || !subchunkHasWater(x, y, layer))
  {
    return;
  }

  HeightData[layer].mHeightValues[y][x] = height;
  HeightData[layer].mHeightValues[y + 1][x] = height;
  HeightData[layer].mHeightValues[y][x + 1] = height;
  HeightData[layer].mHeightValues[y + 1][x + 1] = height;

  reloadRendering();
  */
}

float ChunkWater::getHeight(size_t layer)
{
  /*
  for (size_t y = 0; y < 9; ++y)
  {
    for (size_t x = 0; x < 9; ++x)
    {
      if (subchunkHasWater(x, y, layer))
      {
        return getHeight(x, y, layer);
      }
    }
  }*/
  if (_liquids.size() > layer)
  {
    return _liquids[layer].min();
  }
  return 0.0f;
}

float ChunkWater::getHeight(size_t x, size_t y, size_t layer)
{
  /*
  if (!subchunkHasWater(x, y, layer)) return -1;
  return HeightData[layer].mHeightValues[y][x];
  */
  if (_liquids.size() > layer)
  {
    return _liquids[layer].min();
  }
  return 0.0f;
}

int ChunkWater::getType(size_t layer)
{
  return 0;//return hasData(layer) ? Info[0].LiquidType : 0;
}

void ChunkWater::setType(int type, size_t layer)
{
  /*
  if (!hasData(layer)) return;
  Info[layer].LiquidType = type;
  reloadRendering();*/
}

unsigned char ChunkWater::getTrans(size_t layer)
{
  /*
  for (size_t y = 0; y < 9; ++y)
  {
    for (size_t x = 0; x < 9; ++x)
    {
      if (subchunkHasWater(x, y, layer))
      {
        return getTrans(x, y, layer);
      }
    }
  }
  return 0;
  */
  return 255;
}

unsigned char ChunkWater::getTrans(size_t x, size_t y, size_t layer)
{
  return 255; // subchunkHasWater(x, y, layer) ? HeightData[layer].mTransparency[y][x] : 0;
}

void ChunkWater::setTrans(size_t x, size_t y, unsigned char trans, size_t layer)
{
  /*
  if (!subchunkHasWater(x, y, layer)) return;

  HeightData[layer].mTransparency[y][x] = trans;
  HeightData[layer].mTransparency[y + 1][x] = trans;
  HeightData[layer].mTransparency[y][x + 1] = trans;
  HeightData[layer].mTransparency[y + 1][x + 1] = trans;

  reloadRendering();*/
}

void ChunkWater::setTrans(unsigned char trans, size_t layer)
{
  /*
  for (size_t y = 0; y < 9; ++y)
  {
    for (size_t x = 0; x < 9; ++x)
    {
      HeightData[layer].mTransparency[y][x] = trans;
    }
  }

  reloadRendering();*/
}

bool ChunkWater::hasData(size_t layer)
{
  return true; //(Header.nLayers > layer);
}

void ChunkWater::draw()
{
  if (_liquids.empty())
  {
    return;
  }

  if (Environment::getInstance()->displayAllWaterLayers)
  {
    for (Liquid& liquid : _liquids)
    {
      liquid.draw();
    }
  }
  else if (Environment::getInstance()->currentWaterLayer < _liquids.size())
  {
    _liquids[Environment::getInstance()->currentWaterLayer].draw();
  }
}
