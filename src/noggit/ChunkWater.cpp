// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/liquid_layer.hpp>
#include <noggit/MPQ.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>

ChunkWater::ChunkWater(float x, float z, bool use_mclq_green_lava)
  : xbase(x)
  , zbase(z)
  , vmin(x, 0.f, z)
  , vmax(x + CHUNKSIZE, 0.f, z + CHUNKSIZE)
  , _use_mclq_green_lava(use_mclq_green_lava)
{
}

void ChunkWater::from_mclq(std::vector<mclq>& layers)
{
  math::vector_3d pos(xbase, 0.0f, zbase);
  if (!Render.has_value()) Render.emplace();
  for (mclq& liquid : layers)
  {
    std::uint8_t mclq_liquid_type = 0;

    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        mclq_tile const& tile = liquid.tiles[z * 8 + x];

        misc::bit_or(Render.value().fishable, x, z, tile.fishable);
        misc::bit_or(Render.value().fatigue, x, z, tile.fatigue);

        if (!tile.dont_render)
        {
          mclq_liquid_type = tile.liquid_type;
        }
      }
    }

    switch (mclq_liquid_type)
    {
      case 1:_layers.emplace_back(pos, liquid, 2); break;
      case 3:_layers.emplace_back(pos, liquid, 4); break;
      case 4:_layers.emplace_back(pos, liquid, 1); break;
      case 6:_layers.emplace_back(pos, liquid, (_use_mclq_green_lava ? 15 : 3)); break;
      default:
        LogError << "Invalid/unhandled MCLQ liquid type" << std::endl;
        break;
    }
  }
  update_layers();
}

void ChunkWater::fromFile(MPQFile &f, size_t basePos)
{
  MH2O_Header header;
  f.read(&header, sizeof(MH2O_Header));

  if (!header.nLayers)
  {
    return;
  }

  //render
  if (header.ofsRenderMask)
  {
    Render.emplace();
    f.seek(basePos + header.ofsRenderMask);
    f.read(&Render.value(), sizeof(MH2O_Render));
  }

  for (std::size_t k = 0; k < header.nLayers; ++k)
  {
    MH2O_Information info;
    uint64_t infoMask = 0xFFFFFFFFFFFFFFFF; // default = all water

    //info
    f.seek(basePos + header.ofsInformation + sizeof(MH2O_Information)* k);
    f.read(&info, sizeof(MH2O_Information));

    //mask
    if (info.ofsInfoMask > 0 && info.height > 0)
    {
      size_t bitmask_size = static_cast<size_t>(std::ceil(info.height * info.width / 8.0f));

      f.seek(info.ofsInfoMask + basePos);
      // only read the relevant data
      f.read(&infoMask, bitmask_size);
    }

    math::vector_3d pos(xbase, 0.0f, zbase);
    _layers.emplace_back(f, basePos, pos, info, infoMask);
  }

  update_layers();
}


void ChunkWater::save(util::sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos)
{
  MH2O_Header header;

  // remove empty layers
  cleanup();

  if (hasData(0))
  {
    header.nLayers = _layers.size();

    if (Render.has_value())
    {
        header.ofsRenderMask = current_pos - base_pos;
        adt.Insert(current_pos, sizeof(MH2O_Render), reinterpret_cast<char*>(&Render.value()));
        current_pos += sizeof(MH2O_Render);
    }
    else
    {
        header.ofsRenderMask = 0;
    }

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

  memcpy(adt.GetPointer<char>(header_pos).get(), &header, sizeof(MH2O_Header));
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

void ChunkWater::draw ( math::frustum const& frustum
                      , const float& cull_distance
                      , const math::vector_3d& camera
                      , bool camera_moved
                      , liquid_render& render
                      , opengl::scoped::use_program& water_shader
                      , int animtime
                      , int layer
                      , display_mode display
                      )
{
  if (!is_visible (cull_distance, frustum, camera, display))
  {
    return;
  }

  if (layer == -1)
  {
    for (liquid_layer& lq_layer : _layers)
    {
      lq_layer.draw (render, water_shader, camera, camera_moved, animtime);
    }
  }
  else if (layer < _layers.size())
  {
    _layers[layer].draw (render, water_shader, camera, camera_moved, animtime);
  }
}

bool ChunkWater::is_visible ( const float& cull_distance
                            , const math::frustum& frustum
                            , const math::vector_3d& camera
                            , display_mode display
                            ) const
{
  static const float chunk_radius = std::sqrt (CHUNKSIZE * CHUNKSIZE / 2.0f);

  float dist = display == display_mode::in_3D
             ? (camera - vcenter).length() - chunk_radius
             : std::abs(camera.y - vmax.y);

  return frustum.intersects (_intersect_points)
      && dist < cull_distance;
}

void ChunkWater::update_layers()
{
  for (liquid_layer& layer : _layers)
  {
    layer.update_indices();
    vmin.y = std::min (vmin.y, layer.min());
    vmax.y = std::max (vmax.y, layer.max());
  }

  vcenter = (vmin + vmax) * 0.5f;

  _intersect_points.clear();
  _intersect_points = misc::intersection_points(vmin, vmax);
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
