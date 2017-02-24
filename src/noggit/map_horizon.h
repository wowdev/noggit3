// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/frustum.hpp>

#include <opengl/texture.hpp>
#include <opengl/scoped.hpp>

#include <QtGui/QImage>

#include <memory>

class MapIndex;

struct map_horizon_tile
{
    int16_t height_17[17][17];
    int16_t height_16[16][16];
};

struct map_horizon_batch
{
  map_horizon_batch ()
    : vertex_start (0)
    , vertex_count (0)
  {}

  map_horizon_batch (uint32_t _vertex_start, uint32_t _vertex_count)
    : vertex_start(_vertex_start)
    , vertex_count(_vertex_count)
  {}

  uint32_t vertex_start;
  uint32_t vertex_count;
};

class map_horizon
{
public:
  struct render
  {
    render(const map_horizon& horizon);

    void draw( MapIndex *index
             , const math::vector_3d& color
             , const float& cull_distance
             , const math::frustum& frustum
             , const math::vector_3d& camera );

    map_horizon_batch _batches[64][64];

    opengl::scoped::buffers<2> _buffers;
    GLuint const& _index_buffer = _buffers[0];
    GLuint const& _vertex_buffer = _buffers[1];
  };

  class minimap : public opengl::texture
  {
  public:
    minimap(const map_horizon& horizon);
  };

  map_horizon(const std::string& basename);
  
  QImage _qt_minimap;

private:
  std::string _filename;

  std::unique_ptr<map_horizon_tile> _tiles[64][64];
};