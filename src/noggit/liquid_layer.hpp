// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/liquid_render.hpp>
#include <opengl/scoped.hpp>

class MapChunk;
class sExtendableArray;


// handle liquids like oceans, lakes, rivers, slime, magma
class liquid_layer
{
public:
  liquid_layer() = delete;
  liquid_layer(math::vector_3d const& base, float height, int liquid_id);
  liquid_layer(math::vector_3d const& base, mclq& liquid, int liquid_id);
  liquid_layer(MPQFile &f, std::size_t base_pos, math::vector_3d const& base, MH2O_Information const& info, std::uint64_t infomask);

  liquid_layer(liquid_layer const& other);
  liquid_layer (liquid_layer&&);

  liquid_layer& operator=(liquid_layer&&);
  liquid_layer& operator=(liquid_layer const& other);

  void save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const;

  void draw ( liquid_render& render
            , opengl::scoped::use_program& water_shader
            , math::vector_3d const& camera
            , bool camera_moved
            , int animtime
            );
  void update_indices();
  void changeLiquidID(int id);

  void crop(MapChunk* chunk);
  void update_opacity(MapChunk* chunk, float factor);


  float min() const { return _minimum; }
  float max() const { return _maximum; }
  int liquidID() const { return _liquid_id; }

  bool hasSubchunk(int x, int z, int size = 1) const;
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

// private:
  void update_min_max();
  void update_vertex_opacity(int x, int z, MapChunk* chunk, float factor);
  int get_lod_level(math::vector_3d const& camera_pos) const;
  void set_lod_level(int lod_level);

  static int const lod_count = 4;

  int _current_lod_level = -1;
  int _current_lod_indices_count = 0;

  opengl::scoped::deferred_upload_buffers<lod_count> _index_buffer;
  opengl::scoped::deferred_upload_buffers<3> _buffers;
  GLuint const& _vertices_vbo = _buffers[0];
  GLuint const& _depth_vbo = _buffers[1];
  GLuint const& _tex_coord_vbo = _buffers[2];
  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];

  int _liquid_id;
  int _liquid_vertex_format;
  float _minimum;
  float _maximum;
  std::uint64_t _subchunks;
  std::vector<math::vector_3d> _vertices;
  std::vector<float> _depth;
  std::vector<math::vector_2d> _tex_coords;
  std::map<int, std::vector<std::uint16_t>> _indices_by_lod;

  bool _need_buffer_update = true;
  bool _vao_need_update = true;
  bool _uploaded = false;

  void upload();
  void update_buffers();
  void update_vao(opengl::scoped::vao_binder const& bound_vao, opengl::scoped::use_program& water_shader);

private:
  math::vector_3d pos;
};
