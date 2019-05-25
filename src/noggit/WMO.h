// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.hpp>
#include <math/ray.hpp>
#include <math/vector_3d.hpp>
#include <noggit/MPQ.h>
#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/ModelManager.h>
#include <noggit/multimap_with_normalized_key.hpp>
#include <noggit/TextureManager.h>
#include <noggit/tool_enums.hpp>
#include <noggit/wmo_liquid.hpp>

#include <boost/optional.hpp>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class WMO;
class WMOGroup;
class WMOInstance;
class WMOManager;
class wmo_liquid;
class Model;

struct wmo_batch
{
  int8_t unused[12];

  uint32_t index_start;
  uint16_t index_count;
  uint16_t vertex_start;
  uint16_t vertex_end;

  uint8_t flags;
  uint8_t texture;
};

class WMOGroup {
public:
  WMOGroup(WMO *wmo, MPQFile* f, int num, char const* names);
  WMOGroup(WMOGroup const&);

  void load();

  void draw( opengl::scoped::use_program& wmo_shader
           , math::frustum const& frustum
           , const float& cull_distance
           , const math::vector_3d& camera
           , bool draw_fog
           , bool world_has_skies
           );

  void drawLiquid ( math::matrix_4x4 const& model_view
                  , math::matrix_4x4 const& projection
                  , math::vector_4d const& ocean_color_light
                  , math::vector_4d const& ocean_color_dark
                  , math::vector_4d const& river_color_light
                  , math::vector_4d const& river_color_dark
                  , liquid_render& render
                  , bool draw_fog
                  , int animtime
                  );

  void setupFog (bool draw_fog, std::function<void (bool)> setup_fog);

  void intersect (math::ray const&, std::vector<float>* results) const;

  // todo: portal culling
  bool is_visible( math::vector_3d const& ofs
                 , math::degrees const& angle
                 , math::frustum const& frustum
                 , float const& cull_distance
                 , math::vector_3d const& camera
                 , display_mode display
                 ) const;

  std::vector<uint16_t> doodad_ref() const { return _doodad_ref; }

  math::vector_3d BoundingBoxMin;
  math::vector_3d BoundingBoxMax;
  math::vector_3d VertexBoxMin;
  math::vector_3d VertexBoxMax;

  bool indoor;
  bool hascv;
  bool visible;

  bool outdoorLights;
  std::string name;

private:
  WMO *wmo;
  uint32_t flags;
  ::math::vector_3d center;
  float rad;
  int32_t num;
  int32_t fog;
  std::vector<uint16_t> _doodad_ref;
  std::unique_ptr<wmo_liquid> lq;

  std::vector<wmo_batch> _batches;

  std::vector<::math::vector_3d> _vertices;
  std::vector<::math::vector_3d> _normals;
  std::vector<::math::vector_2d> _texcoords;
  std::vector<::math::vector_4d> _vertex_colors;
  std::vector<uint16_t> _indices;

  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  opengl::scoped::deferred_upload_buffers<4> _buffers;
  GLuint const& _vertices_buffer = _buffers[0];
  GLuint const& _normals_buffer = _buffers[1];
  GLuint const& _texcoords_buffer = _buffers[2];
  GLuint const& _vertex_colors_buffer = _buffers[3];

  bool _uploaded = false;
  bool _vao_is_setup = false;

  void upload();
  void setup_vao(opengl::scoped::use_program& wmo_shader);
};

struct WMOLight {
  uint32_t flags, color;
  math::vector_3d pos;
  float intensity;
  float unk[5];
  float r;

  math::vector_4d fcolor;

  void init(MPQFile* f);
  void setup(GLint light);

  static void setupOnce(GLint light, math::vector_3d dir, math::vector_3d light_color);
};

struct WMOPV {
  math::vector_3d a, b, c, d;
};

struct WMOPR {
  int16_t portal, group, dir, reserved;
};

struct WMODoodadSet {
  char name[0x14];
  int32_t start;
  int32_t size;
  int32_t unused;
};

struct WMOFog {
  unsigned int flags;
  math::vector_3d pos;
  float r1, r2, fogend, fogstart;
  unsigned int color1;
  float f2;
  float f3;
  unsigned int color2;
  // read to here (0x30 bytes)
  math::vector_4d color;
  void init(MPQFile* f);
  void setup();
};

class WMO : public AsyncObject
{
public:
  explicit WMO(const std::string& name);

  void draw ( opengl::scoped::use_program& wmo_shader
            , math::matrix_4x4 const& model_view
            , math::matrix_4x4 const& projection
            , math::matrix_4x4 const& transform
            , int doodadset
            , const math::vector_3d& ofs
            , math::degrees const
            , bool boundingbox
            , math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool draw_doodads
            , bool draw_fog
            , math::vector_4d const& ocean_color_light
            , math::vector_4d const& ocean_color_dark
            , math::vector_4d const& river_color_light
            , math::vector_4d const& river_color_dark
            , liquid_render& render
            , int animtime
            , bool world_has_skies
            , display_mode display
            );
  bool drawSkybox ( math::vector_3d pCamera
                  , math::vector_3d pLower
                  , math::vector_3d pUpper
                  , bool draw_fog
                  , int animtime
                  ) const;

  std::vector<float> intersect (math::ray const&) const;

  void finishLoading();

  std::map<uint32_t, std::vector<wmo_doodad_instance>> doodads_per_group(uint16_t doodadset) const;

  bool draw_group_boundingboxes;

  bool _finished_upload;

  std::vector<WMOGroup> groups;
  std::vector<WMOMaterial> materials;
  math::vector_3d extents[2];
  std::vector<scoped_blp_texture_reference> textures;
  std::vector<std::string> models;
  std::vector<wmo_doodad_instance> modelis;
  std::vector<math::vector_3d> model_nearest_light_vector;

  std::vector<WMOLight> lights;

  std::vector<WMOFog> fogs;

  std::vector<WMODoodadSet> doodadsets;

  boost::optional<scoped_model_reference> skybox;

  bool is_hidden() const { return _hidden; }
  void toggle_visibility() { _hidden = !_hidden; }
  void show() { _hidden = false ; }

  virtual bool is_required_when_saving() const
  {
    return true;
  }

private:
  bool _hidden = false;
};

class WMOManager
{
public:
  static void report();
  static void clear_hidden_wmos();
private:
  friend struct scoped_wmo_reference;
  static noggit::async_object_multimap_with_normalized_key<WMO> _;
};

struct scoped_wmo_reference
{
  scoped_wmo_reference (std::string const& filename)
    : _valid (true)
    , _filename (filename)
    , _wmo (WMOManager::_.emplace (_filename))
  {}

  scoped_wmo_reference (scoped_wmo_reference const& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _wmo (WMOManager::_.emplace (_filename))
  {}
  scoped_wmo_reference& operator= (scoped_wmo_reference const& other)
  {
    _valid = other._valid;
    _filename = other._filename;
    _wmo = WMOManager::_.emplace (_filename);
    return *this;
  }

  scoped_wmo_reference (scoped_wmo_reference&& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _wmo (other._wmo)
  {
    other._valid = false;
  }
  scoped_wmo_reference& operator= (scoped_wmo_reference&& other)
  {
    std::swap(_valid, other._valid);
    std::swap(_filename, other._filename);
    std::swap(_wmo, other._wmo);
    other._valid = false;
    return *this;
  }

  ~scoped_wmo_reference()
  {
    if (_valid)
    {
      WMOManager::_.erase (_filename);
    }
  }

  WMO* operator->() const
  {
    return _wmo;
  }
  WMO* get() const
  {
    return _wmo;
  }

private:  
  bool _valid;

  std::string _filename;
  WMO* _wmo;
};
