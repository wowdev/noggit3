// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/Environment.h>
#include <noggit/Liquid.h>
#include <noggit/MPQ.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>

ChunkWater::ChunkWater(float x, float z)
  : xbase(x)
  , zbase(z)
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
    MH2O_Information info;
    MH2O_HeightMask heightmask;
    uint64_t infoMask = -1; // default = all water

    //info
    f.seek(basePos + header.ofsInformation + sizeof(MH2O_Information)* k);
    f.read(&info, sizeof(MH2O_Information));

    //mask
    if (info.ofsInfoMask > 0 && info.height > 0)
    {
      f.seek(info.ofsInfoMask + basePos);
      f.read(&infoMask, 8);
    }
    
    // set default value
    for (int h = 0; h < 9; ++h)
    {
      for (int w = 0; w < 9; ++w)
      {
        heightmask.mHeightValues[w][h] = info.minHeight;
        heightmask.mTransparency[w][h] = 255;
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
            f.read(&heightmask.mHeightValues[w][h], sizeof(float));
          }
        }
      }

      for (int w = info.yOffset; w < info.yOffset + info.height + 1; ++w)
      {
        for (int h = info.xOffset; h < info.xOffset + info.width + 1; ++h)
        {
          
          f.read(&heightmask.mTransparency[w][h], sizeof(unsigned char));
          
        }
      }
    }

    _liquids.emplace_back(math::vector_3d(xbase, 0.0f, zbase), info, heightmask, infoMask);
  }
}

void ChunkWater::reloadRendering()
{
  for (Liquid& layer : _liquids)
  {
    layer.updateRender();
  }
}

void ChunkWater::save(sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos)
{
  MH2O_Header header;
  header.nLayers = _liquids.size();

  if (hasData(0))
  {
    header.ofsRenderMask = current_pos - base_pos;
    adt.Insert(current_pos, sizeof(MH2O_Render), reinterpret_cast<char*>(&Render));
    current_pos += sizeof(MH2O_Render);

    header.ofsInformation = current_pos - base_pos;
    int info_pos = current_pos;

    std::size_t info_size = sizeof(MH2O_Information) * _liquids.size();
    current_pos += info_size;

    adt.Extend(info_size);

    for (Liquid& liquid : _liquids)
    {
      liquid.save(adt, base_pos, info_pos, current_pos);
    }
  }  

  memcpy(adt.GetPointer<char>(header_pos), &header, sizeof(MH2O_Header));
  header_pos += sizeof(MH2O_Header);
}


void ChunkWater::autoGen(MapChunk *chunk, float factor)
{
  for (Liquid& liquid : _liquids)
  {
    liquid.updateTransparency(chunk, factor);
  }
}


void ChunkWater::CropWater(MapChunk* chunkTerrain)
{
  for (Liquid& liquid : _liquids)
  {
    liquid.crop(chunkTerrain);
  }
}

int ChunkWater::getType(size_t layer) const
{
  return hasData(layer) ? _liquids[layer].liquidID() : 0;
}

void ChunkWater::setType(int type, size_t layer)
{
  if(hasData(layer))
  {
    _liquids[layer].changeLiquidID(type);
  }
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

bool ChunkWater::hasData(size_t layer) const
{ 
  return _liquids.size() > layer; 
}
