// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/Environment.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/MPQ.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>

ChunkWater::ChunkWater(float x, float z)
  : xbase(x)
  , zbase(z)
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

    _layers.emplace_back(math::vector_3d(xbase, 0.0f, zbase), info, heightmask, infoMask);
  }
}


void ChunkWater::save(sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos)
{
  MH2O_Header header;
  header.nLayers = _layers.size();

  if (hasData(0))
  {
    header.ofsRenderMask = current_pos - base_pos;
    adt.Insert(current_pos, sizeof(MH2O_Render), reinterpret_cast<char*>(&Render));
    current_pos += sizeof(MH2O_Render);

    header.ofsInformation = current_pos - base_pos;
    int info_pos = current_pos;

    std::size_t info_size = sizeof(MH2O_Information) * _layers.size();
    current_pos += info_size;

    adt.Extend(info_size);

    for (liquid_layer& layer : _layers)
    {
      layer.save(adt, base_pos, info_pos, current_pos);
    }
  }

  memcpy(adt.GetPointer<char>(header_pos), &header, sizeof(MH2O_Header));
  header_pos += sizeof(MH2O_Header);
}


void ChunkWater::autoGen(MapChunk *chunk, float factor)
{
  for (liquid_layer& layer : _layers)
  {
    layer.updateTransparency(chunk, factor);
  }
}


void ChunkWater::CropWater(MapChunk* chunkTerrain)
{
  for (liquid_layer& layer : _layers)
  {
    layer.crop(chunkTerrain);
  }
}

int ChunkWater::getType(size_t layer) const
{
  return hasData(layer) ? _layers[layer].liquidID() : 0;
}

void ChunkWater::setType(int type, size_t layer)
{
  if(hasData(layer))
  {
    _layers[layer].changeLiquidID(type);
  }
}

void ChunkWater::draw()
{
  if (_layers.empty())
  {
    return;
  }

  if (Environment::getInstance()->displayAllWaterLayers)
  {
    for (liquid_layer& layer : _layers)
    {
      layer.draw();
    }
  }
  else if (Environment::getInstance()->currentWaterLayer < _layers.size())
  {
    _layers[Environment::getInstance()->currentWaterLayer].draw();
  }
}

bool ChunkWater::hasData(size_t layer) const
{
  return _layers.size() > layer;
}


void ChunkWater::paintLiquid(math::vector_3d const& pos, float radius, int liquid_id, bool add)
{
  bool painted = false;
  for (liquid_layer& layer : _layers)
  {
    // remove the water on all layers or paint the layer with selected id
    if (!add || layer.liquidID() == liquid_id)
    {
      layer.paintLiquid(pos, radius, add);
      painted = true;
    }
    else
    {
      layer.paintLiquid(pos, radius, false);
    }
  }

  cleanup();

  if (!add || painted)
  {
    return;
  }
  
  if (hasData(0))
  {
    // wow only suport 5 layers
    if (_layers.size() < 5)
    {
      liquid_layer layer(_layers[0]);
      layer.clear(); // remove the liquid to not override the other layer
      layer.paintLiquid(pos, radius, true);
      layer.changeLiquidID(liquid_id);
      _layers.push_back(layer);
    }
  }
  else
  {
    // had new layer at just above the cursor
    liquid_layer layer(math::vector_3d(xbase, 0.0f, zbase), pos.y + 1.0f, liquid_id);
    layer.paintLiquid(pos, radius, true);
    _layers.push_back(layer);
  }
}

void ChunkWater::cleanup()
{
  for (int i = _layers.size() - 1; i >= 0; --i)
  {
    if (_layers[i].empty())
    {
      _layers.erase(_layers.begin() + i);
    }
  }
}
