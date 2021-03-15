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

union wmo_group_flags
{
  uint32_t value;
  struct
  {
    uint32_t has_bsp_tree : 1; // 0x1
    uint32_t has_light_map : 1; // 0x2
    uint32_t has_vertex_color : 1; // 0x4
    uint32_t exterior : 1; // 0x8
    uint32_t flag_0x10 : 1;
    uint32_t flag_0x20 : 1;
    uint32_t exterior_lit : 1; // 0x40
    uint32_t unreacheable : 1; // 0x80
    uint32_t flag_0x100: 1;
    uint32_t has_light : 1; // 0x200
    uint32_t flag_0x400 : 1;
    uint32_t has_doodads : 1; // 0x800
    uint32_t has_water : 1; // 0x1000
    uint32_t indoor : 1; // 0x2000
    uint32_t flag_0x4000 : 1;
    uint32_t flag_0x8000 : 1;
    uint32_t always_draw : 1; // 0x10000
    uint32_t has_mori_morb : 1; // 0x20000, cata+ only (?)
    uint32_t skybox : 1; // 0x40000
    uint32_t ocean : 1; // 0x80000
    uint32_t flag_0x100000 : 1;
    uint32_t mount_allowed : 1; // 0x200000
    uint32_t flag_0x400000 : 1;
    uint32_t flag_0x800000 : 1;
    uint32_t use_mocv2_for_texture_blending : 1; // 0x1000000
    uint32_t has_two_motv : 1; // 0x2000000
    uint32_t antiportal : 1; // 0x4000000
    uint32_t unk : 1; // 0x8000000 requires intBatchCount == 0, extBatchCount == 0, UNREACHABLE. 
    uint32_t unused : 4;
  };
};
static_assert ( sizeof (wmo_group_flags) == sizeof (std::uint32_t)
              , "bitfields shall be implemented packed"
              );

struct wmo_group_header
{
  uint32_t group_name; // offset into MOGN
  uint32_t descriptive_group_name; // offset into MOGN
  wmo_group_flags flags;
  float box1[3];
  float box2[3];
  uint16_t portal_start;
  uint16_t portal_count;
  uint16_t transparency_batches_count;
  uint16_t interior_batch_count;
  uint16_t exterior_batch_count;
  uint16_t padding_or_batch_type_d; // probably padding, but might be data?
  uint8_t fogs[4];
  uint32_t group_liquid; // used for MLIQ
  uint32_t id;
  int32_t unk2, unk3;
};

struct wmo_group_uniform_data
{
  int mocv = -1;
  int exterior_lit = -1;
  int blend_mode = -1;
  int shader = -1;
  int unfogged = -1;
  int unlit = -1;
};

class WMOGroup 
{
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
           , wmo_group_uniform_data& wmo_uniform_data
           );

  void drawLiquid ( math::matrix_4x4 const& transform
                  , liquid_render& render
                  , bool draw_fog
                  , int animtime
                  );

  void setupFog (bool draw_fog, std::function<void (bool)> setup_fog);

  void intersect (math::ray const&, std::vector<float>* results) const;

  // todo: portal culling
  bool is_visible( math::matrix_4x4 const& transform_matrix
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

  bool use_outdoor_lights;
  std::string name;

  bool has_skybox() const { return header.flags.skybox; }

private:
  void load_mocv(MPQFile& f, uint32_t size);
  void fix_vertex_color_alpha();

  WMO *wmo;
  wmo_group_header header;
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
  std::vector<::math::vector_2d> _texcoords_2;
  std::vector<::math::vector_4d> _vertex_colors;
  std::vector<uint16_t> _indices;

  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  opengl::scoped::deferred_upload_buffers<6> _buffers;
  GLuint const& _vertices_buffer = _buffers[0];
  GLuint const& _normals_buffer = _buffers[1];
  GLuint const& _texcoords_buffer = _buffers[2];
  GLuint const& _texcoords_buffer_2 = _buffers[3];
  GLuint const& _vertex_colors_buffer = _buffers[4];
  GLuint const& _indices_buffer = _buffers[5];

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

union mohd_flags
{
  std::uint16_t flags;
  struct
  {
    std::uint16_t do_not_attenuate_vertices_based_on_distance_to_portal : 1;
    std::uint16_t use_unified_render_path : 1;
    std::uint16_t use_liquid_type_dbc_id : 1;
    std::uint16_t do_not_fix_vertex_color_alpha : 1;
    std::uint16_t unused : 12;
  };
};
static_assert ( sizeof (mohd_flags) == sizeof (std::uint16_t)
              , "bitfields shall be implemented packed"
              );

class WMO : public AsyncObject
{
public:
  explicit WMO(const std::string& name);

  void draw ( opengl::scoped::use_program& wmo_shader
            , math::matrix_4x4 const& model_view
            , math::matrix_4x4 const& projection
            , math::matrix_4x4 const& transform_matrix
            , math::matrix_4x4 const& transform_matrix_transposed
            , bool boundingbox
            , math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool draw_doodads
            , bool draw_fog
            , liquid_render& render
            , int animtime
            , bool world_has_skies
            , display_mode display
            , wmo_group_uniform_data& wmo_uniform_data
            );
  bool draw_skybox( math::matrix_4x4 const& model_view
                  , math::vector_3d const& camera_pos
                  , opengl::scoped::use_program& m2_shader
                  , math::frustum const& frustum
                  , const float& cull_distance
                  , int animtime
                  , bool draw_particles
                  , math::vector_3d aabb_min
                  , math::vector_3d aabb_max
                  , std::map<int, std::pair<math::vector_3d, math::vector_3d>> const& group_extents
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
  math::vector_4d ambient_light_color;

  mohd_flags flags;

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
