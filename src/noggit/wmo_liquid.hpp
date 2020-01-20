// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

// #define USEBLSFILES

#include <math/vector_2d.hpp>
#include <math/vector_3d.hpp>
#include <noggit/MPQ.h>
#include <noggit/TextureManager.h>
#include <noggit/liquid_render.hpp>
#include <opengl/scoped.hpp>

#include <boost/optional.hpp>

#include <memory>

struct CImVector
{
  std::uint8_t b;
  std::uint8_t g;
  std::uint8_t r;
  std::uint8_t a;
};

struct CArgb
{
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
  std::uint8_t a;
};

struct SMOLTile
{
  uint8_t liquid : 6;
  uint8_t fishable : 1;
  uint8_t shared : 1;
};

struct WMOMaterial 
{
  union
  {
    uint32_t value;
    struct
    {
      uint32_t unlit :  1;
      uint32_t unfogged : 1;
      uint32_t unculled : 1;
      uint32_t ext_light: 1; // darkened used for the intern face of windows
      uint32_t sidn :  1;
      uint32_t window :  1; // lighting related(flag checked in CMapObj::UpdateSceneMaterials)
      uint32_t clamp_s :  1;
      uint32_t clamp_t : 1;
      uint32_t unused : 24;
    };
  } flags;
  uint32_t shader;
  uint32_t blend_mode; // Blending: 0 for opaque, 1 for transparent
  uint32_t texture_offset_1; // Start position for the first texture filename in the MOTX data block
  CImVector sidn_color; // emissive color
  CImVector frame_sidn_color; // runtime value
  uint32_t texture_offset_2; // Start position for the second texture filename in the MOTX data block
  CArgb diffuse_color;
  uint32_t ground_type;
  uint32_t texture_offset_3; 
  uint32_t color_2;
  uint32_t flag_2;
  uint32_t runtime_data[2];
  // also runtime data
  uint32_t texture1; // this is the first texture object.
  uint32_t texture2; // this is the second texture object.
};

struct WMOLiquidHeader {
  int32_t X, Y, A, B;
  math::vector_3d pos;
  int16_t material_id;
};

struct SMOWVert
{
  std::uint8_t flow1;
  std::uint8_t flow2;
  std::uint8_t flow1Pct;
  std::uint8_t filler;
};
struct SMOMVert
{
  std::int16_t s;
  std::int16_t t;
};

struct LiquidVertex {
  union
  {
    SMOWVert water_vertex;
    SMOMVert magma_vertex;
  };
  float height;
};

class wmo_liquid
{
public:
  wmo_liquid(MPQFile* f, WMOLiquidHeader const& header, WMOMaterial const& mat, int group_liquid, bool use_dbc_type, bool is_ocean);
  wmo_liquid(wmo_liquid const& other);

  void draw ( math::matrix_4x4 const& transform
            , liquid_render& render
            , int animtime
            );

  void upload(opengl::scoped::use_program& water_shader);

private:
  int initGeometry(MPQFile* f);

  math::vector_3d pos;
  bool mTransparency;
  int xtiles, ytiles;
  int _liquid_id;

  std::vector<float> depths;
  std::vector<math::vector_2d> tex_coords;
  std::vector<math::vector_3d> vertices;
  std::vector<std::uint16_t> indices;

  int _indices_count;

  bool _uploaded = false;

  opengl::scoped::deferred_upload_buffers<4> _buffer;
  GLuint const& _indices_buffer = _buffer[0];
  GLuint const& _vertices_buffer = _buffer[1];
  GLuint const& _depth_buffer = _buffer[2];
  GLuint const& _tex_coord_buffer = _buffer[3];
  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
};
