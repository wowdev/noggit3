// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/frustum.hpp>
#include <math/matrix_4x4.hpp>
#include <math/quaternion.hpp>
#include <math/ray.hpp>
#include <math/vector_3d.hpp>
#include <noggit/Animated.h> // Animation::M2Value
#include <noggit/AsyncObject.h> // AsyncObject
#include <noggit/MPQ.h>
#include <noggit/ModelHeaders.h>
#include <noggit/Particle.h>
#include <noggit/TextureManager.h>
#include <noggit/tool_enums.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.fwd.hpp>

#include <string>
#include <vector>

class Bone;
class Model;
class ModelInstance;
class ParticleSystem;
class RibbonEmitter;

math::vector_3d fixCoordSystem(math::vector_3d v);

class Bone {
  Animation::M2Value<math::vector_3d> trans;
  Animation::M2Value<math::quaternion, math::packed_quaternion> rot;
  Animation::M2Value<math::vector_3d> scale;

public:
  math::vector_3d pivot;
  int parent;

  typedef struct
  {
    uint32_t flag_0x1 : 1;
    uint32_t flag_0x2 : 1;
    uint32_t flag_0x4 : 1;
    uint32_t billboard : 1;
    uint32_t cylindrical_billboard_lock_x : 1;
    uint32_t cylindrical_billboard_lock_y : 1;
    uint32_t cylindrical_billboard_lock_z : 1;
    uint32_t flag_0x80 : 1;
    uint32_t flag_0x100 : 1;
    uint32_t transformed : 1;
    uint32_t unused : 20;
  } bone_flags;

  bone_flags flags;
  math::matrix_4x4 mat = math::matrix_4x4::uninitialized;
  math::matrix_4x4 mrot = math::matrix_4x4::uninitialized;

  bool calc;
  void calcMatrix( math::matrix_4x4 const& model_view
                 , Bone* allbones
                 , int anim
                 , int time
                 , int animtime
                 );
  Bone ( const MPQFile& f,
         const ModelBoneDef &b,
         int *global,
         const std::vector<std::unique_ptr<MPQFile>>& animation_files
       );

};


class TextureAnim {
  Animation::M2Value<math::vector_3d> trans;
  Animation::M2Value<math::quaternion, math::packed_quaternion> rot;
  Animation::M2Value<math::vector_3d> scale;

public:
  math::matrix_4x4 mat;

  void calc(int anim, int time, int animtime);
  TextureAnim(const MPQFile& f, const ModelTexAnimDef &mta, int *global);
};

struct ModelColor {
  Animation::M2Value<math::vector_3d> color;
  Animation::M2Value<float, int16_t> opacity;

  ModelColor(const MPQFile& f, const ModelColorDef &mcd, int *global);
};

struct ModelTransparency {
  Animation::M2Value<float, int16_t> trans;

  ModelTransparency(const MPQFile& f, const ModelTransDef &mtd, int *global);
};


enum class M2Blend : uint16_t
{
  Opaque,
  Alpha_Key,
  Alpha,
  No_Add_Alpha,
  Add,
  Mod,
  Mod2x
};

enum class ModelPixelShader : uint16_t
{
  Combiners_Opaque,
  Combiners_Decal,
  Combiners_Add,
  Combiners_Mod2x,
  Combiners_Fade,
  Combiners_Mod,
  Combiners_Opaque_Opaque,
  Combiners_Opaque_Add,
  Combiners_Opaque_Mod2x,
  Combiners_Opaque_Mod2xNA,
  Combiners_Opaque_AddNA,
  Combiners_Opaque_Mod,
  Combiners_Mod_Opaque,
  Combiners_Mod_Add,
  Combiners_Mod_Mod2x,
  Combiners_Mod_Mod2xNA,
  Combiners_Mod_AddNA,
  Combiners_Mod_Mod,
  Combiners_Add_Mod,
  Combiners_Mod2x_Mod2x,
  Combiners_Opaque_Mod2xNA_Alpha,
  Combiners_Opaque_AddAlpha,
  Combiners_Opaque_AddAlpha_Alpha,
};

enum class texture_unit_lookup : int
{
  environment,
  t1,
  t2,
  none
};


struct ModelRenderPass : ModelTexUnit
{
  ModelRenderPass() = delete;
  ModelRenderPass(ModelTexUnit const& tex_unit, Model* m);

  float ordering_thingy = 0.f;
  uint16_t index_start = 0, index_count = 0, vertex_start = 0, vertex_end = 0;
  uint16_t blend_mode = 0;
  texture_unit_lookup tu_lookups[2];
  uint16_t textures[2];
  uint16_t uv_animations[2];
  boost::optional<ModelPixelShader> pixel_shader;


  bool prepare_draw(opengl::scoped::use_program& m2_shader, Model *m);
  void after_draw();
  void bind_texture(size_t index, Model* m);
  void init_uv_types(Model* m);

  bool operator< (const ModelRenderPass &m) const
  {
    if (priority_plane < m.priority_plane)
    {
      return true;
    }
    else if (priority_plane > m.priority_plane) 
    { 
      return false; 
    }
    else
    {
      return blend_mode == m.blend_mode ? (ordering_thingy < m.ordering_thingy) : blend_mode < m.blend_mode;
    }
  }
};

struct FakeGeometry
{
  FakeGeometry(Model* m);

  std::vector<math::vector_3d> vertices;
  std::vector<uint16_t> indices;
};

struct ModelLight {
  int type, parent;
  math::vector_3d pos, tpos, dir, tdir;
  Animation::M2Value<math::vector_3d> diffColor, ambColor;
  Animation::M2Value<float> diffIntensity, ambIntensity;
  //Animation::M2Value<float> attStart,attEnd;
  //Animation::M2Value<bool> Enabled;

  ModelLight(const MPQFile&  f, const ModelLightDef &mld, int *global);
  void setup(int time, opengl::light l, int animtime);
};

class Model : public AsyncObject
{
public:
  template<typename T>
    static std::vector<T> M2Array(MPQFile const& f, uint32_t offset, uint32_t count)
  {
    T const* start = reinterpret_cast<T const*>(f.getBuffer() + offset);
    return std::vector<T>(start, start + count);
  }

  Model(const std::string& name);

  void draw( math::matrix_4x4 const& model_view
           , ModelInstance& instance
           , opengl::scoped::use_program& m2_shader
           , math::frustum const& frustum
           , const float& cull_distance
           , const math::vector_3d& camera
           , int animtime
           , bool draw_particles
           , bool all_boxes
           , display_mode display
           );
  void draw ( math::matrix_4x4 const& model_view
            , std::vector<ModelInstance*> instances
            , opengl::scoped::use_program& m2_shader
            , math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool draw_fog
            , int animtime
            , bool draw_particles
            , bool all_boxes
            , std::unordered_map<Model*, std::size_t>& models_with_particles
            , std::unordered_map<Model*, std::size_t>& model_boxes_to_draw
            , display_mode display
            );
  void draw_particles( math::matrix_4x4 const& model_view
                     , opengl::scoped::use_program& particles_shader
                     , std::size_t instance_count
                     );
  void draw_ribbons( opengl::scoped::use_program& ribbons_shader
                   , std::size_t instance_count
                   );

  void draw_box (opengl::scoped::use_program& m2_box_shader, std::size_t box_count);

  std::vector<float> intersect (math::matrix_4x4 const& model_view, math::ray const&, int animtime);

  void updateEmitters(float dt);

  virtual void finishLoading();

  bool is_hidden() const { return _hidden; }
  void toggle_visibility() { _hidden = !_hidden; }
  void show() { _hidden = false ; }

  bool use_fake_geometry() const { return !!_fake_geometry; }

  virtual bool is_required_when_saving() const
  {
    return true;
  }

  // ===============================
  // Toggles
  // ===============================
  std::vector<bool> showGeosets;

  // ===============================
  // Texture data
  // ===============================
  std::vector<scoped_blp_texture_reference> _textures;
  std::vector<std::string> _textureFilenames;
  std::map<std::size_t, scoped_blp_texture_reference> _replaceTextures;
  std::vector<int> _specialTextures;
  std::vector<bool> _useReplaceTextures;
  std::vector<int16_t> _texture_unit_lookup;

  // ===============================
  // Misc ?
  // ===============================
  std::vector<Bone> bones;
  ModelHeader header;
  std::vector<uint16_t> blend_override;

  float rad;
  float trans;
  bool animcalc;  

private:
  bool _per_instance_animation;
  int _current_anim_seq;
  int _anim_time;
  int _global_animtime;

  void initCommon(const MPQFile& f);
  bool isAnimated(const MPQFile& f);
  void initAnimated(const MPQFile& f);

  void fix_shader_id_blend_override();
  void fix_shader_id_layer();
  void compute_pixel_shader_ids();

  void animate(math::matrix_4x4 const& model_view, int anim_id, int anim_time);
  void calcBones(math::matrix_4x4 const& model_view, int anim, int time, int animation_time);

  void lightsOn(opengl::light lbase);
  void lightsOff(opengl::light lbase);

  void upload();

  bool _finished_upload;

  std::vector<math::vector_3d> _vertex_box_points;

  // buffers;
  opengl::scoped::deferred_upload_buffers<3> _buffers;
  opengl::scoped::deferred_upload_vertex_arrays<2> _vertex_arrays;

  GLuint const& _vao = _vertex_arrays[0];
  GLuint const& _transform_buffer = _buffers[0];
  GLuint const& _vertices_buffer = _buffers[1];

  GLuint const& _box_vao = _vertex_arrays[1];
  GLuint const& _box_vbo = _buffers[2];

  // ===============================
  // Geometry
  // ===============================

  std::vector<ModelVertex> _vertices;
  std::vector<ModelVertex> _current_vertices;

  std::vector<uint16_t> _indices;

  std::vector<ModelRenderPass> _render_passes;
  boost::optional<FakeGeometry> _fake_geometry;

  // ===============================
  // Animation
  // ===============================
  bool animated;
  bool animGeometry, animTextures, animBones;

  //      <anim_id, <sub_anim_id, animation>
  std::map<uint16_t, std::map<uint16_t, ModelAnimation>> _animations_seq_per_id;
  std::map<int16_t, uint32_t> _animation_length;

  std::vector<ModelRenderFlags> _render_flags;
  std::vector<ParticleSystem> _particles;
  std::vector<RibbonEmitter> _ribbons;

  std::vector<int> _global_sequences;
  std::vector<TextureAnim> _texture_animations;
  std::vector<int16_t> _texture_animation_lookups;
  std::vector<uint16_t> _texture_lookup;

  // ===============================
  // Material
  // ===============================
  std::vector<ModelColor> _colors;
  std::vector<ModelTransparency> _transparency;
  std::vector<int16_t> _transparency_lookup;
  std::vector<ModelLight> _lights;

  bool _hidden = false;

  friend struct ModelRenderPass;
};

