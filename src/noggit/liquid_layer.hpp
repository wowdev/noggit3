// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/liquid_render.hpp>
#include <noggit/MapHeaders.h>
#include <opengl/scoped.hpp>

class MapChunk;
class sExtendableArray;


// handle liquids like oceans, lakes, rivers, slime, magma
class liquid_layer
{
public:

  liquid_layer(math::vector_3d const& base, float height, int liquid_id);
  liquid_layer(math::vector_3d const& base, MH2O_Information const& info, MH2O_HeightMask const& heightmask, std::uint64_t infomask);
  liquid_layer(liquid_layer const& other);

  liquid_layer& operator=(liquid_layer const& other);

  void save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const;

  void draw (opengl::scoped::use_program& water_shader);
  void updateRender();
  void changeLiquidID(int id);
  
  void crop(MapChunk* chunk);
  void update_opacity(MapChunk* chunk, float factor);


  float min() const { return _minimum; }
  float max() const { return _maximum; }
  int liquidID() const { return _liquid_id; }

  bool hasSubchunk(int x, int z) const;
  void setSubchunk(int x, int z, bool water);

  bool empty() const { return !_subchunks; }
  bool full() const { return _subchunks == std::uint64_t(-1); }
  void clear() { _subchunks = std::uint64_t(0); }

  void paintLiquid(math::vector_3d const& pos
                  , float radius
                  , bool add
                  , math::radians const& angle
                  , math::radians const& orientation
                  , bool lock
                  , math::vector_3d const& origin
                  , bool override_height
                  , MapChunk* chunk
                  , float opacity_factor
                  );

  void copy_subchunk_height(int x, int z, liquid_layer const& from);

private:
  void update_min_max();
  void update_vertex_opacity(int x, int z, MapChunk* chunk, float factor);

  opengl::scoped::buffers<1> _index_buffer;
  std::vector<float> depths;
  std::vector<math::vector_2d> tex_coords;
  std::vector<math::vector_3d> vertices;
  std::vector<std::uint16_t> indices;
  void draw_actual (opengl::scoped::use_program&);

  int _liquid_id;
  int _liquid_vertex_format;
  float _minimum;
  float _maximum;
  std::uint64_t _subchunks;
  std::vector<math::vector_3d> _vertices;
  std::vector<float> _depth;

private:
  math::vector_3d pos;
  float texRepeats;

  liquid_render _render;
};
