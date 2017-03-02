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

      if (info.liquid_vertex_format != 2)
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

  update_layers();
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
    layer.update_opacity(chunk, factor);
  }
  update_layers();
}


void ChunkWater::CropWater(MapChunk* chunkTerrain)
{
  for (liquid_layer& layer : _layers)
  {
    layer.crop(chunkTerrain);
  }
  update_layers();
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

void ChunkWater::draw ( opengl::scoped::use_program& water_shader
                      , math::vector_3d water_color_light
                      , math::vector_3d water_color_dark
                      )
{
  if (_layers.empty())
  {
    return;
  }

  if (Environment::getInstance()->displayAllWaterLayers)
  {
    for (liquid_layer& layer : _layers)
    {
      layer.draw ( water_shader
                 , water_color_light
                 , water_color_dark
                 );
    }
  }
  else if (Environment::getInstance()->currentWaterLayer < _layers.size())
  {
    _layers[Environment::getInstance()->currentWaterLayer].draw
      (water_shader, water_color_light, water_color_dark);
  }
}

void ChunkWater::update_layers()
{
  for (liquid_layer& layer : _layers)
  {
    layer.updateRender();
  }
}

bool ChunkWater::hasData(size_t layer) const
{
  return _layers.size() > layer;
}


void ChunkWater::paintLiquid( math::vector_3d const& pos
                            , float radius
                            , int liquid_id
                            , bool add
                            , math::radians const& angle
                            , math::radians const& orientation
                            , bool lock
                            , math::vector_3d const& origin
                            , bool override_height
                            , bool override_liquid_id
                            , MapChunk* chunk
                            , float opacity_factor
                            )
{
  if (override_liquid_id && !override_height)
  {
    bool layer_found = false;
    for (liquid_layer& layer : _layers)
    {
      if (layer.liquidID() == liquid_id)
      {
        copy_height_to_layer(layer, pos, radius);
        layer_found = true;
        break;
      }
    }

    if (!layer_found)
    {
      liquid_layer layer(math::vector_3d(xbase, 0.0f, zbase), pos.y, liquid_id);
      copy_height_to_layer(layer, pos, radius);
      _layers.push_back(layer);
    }
  }

  bool painted = false;
  for (liquid_layer& layer : _layers)
  {
    // remove the water on all layers or paint the layer with selected id
    if (!add || layer.liquidID() == liquid_id || !override_liquid_id)
    {
      layer.paintLiquid(pos, radius, add, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
      painted = true;
    }
    else
    {
      layer.paintLiquid(pos, radius, false, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
    }
  }

  cleanup();

  if (!add || painted)
  {
    update_layers();
    return;
  }

  if (hasData(0))
  {
    liquid_layer layer(_layers[0]);
    layer.clear(); // remove the liquid to not override the other layer
    layer.paintLiquid(pos, radius, true, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
    layer.changeLiquidID(liquid_id);
    _layers.push_back(layer);
  }
  else
  {
    liquid_layer layer(math::vector_3d(xbase, 0.0f, zbase), pos.y, liquid_id);
    layer.paintLiquid(pos, radius, true, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
    _layers.push_back(layer);
  }

  update_layers();
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

void ChunkWater::copy_height_to_layer(liquid_layer& target, math::vector_3d const& pos, float radius)
{
  for (liquid_layer& layer : _layers)
  {
    if (layer.liquidID() == target.liquidID())
    {
      continue;
    }

    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        if (misc::getShortestDist(pos.x, pos.z, xbase + x*UNITSIZE, zbase + z*UNITSIZE, UNITSIZE) <= radius)
        {
          if (layer.hasSubchunk(x, z))
          {
            target.copy_subchunk_height(x, z, layer);
          }
        }
      }
    }
  }
}
