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

  bool billboard;
  math::matrix_4x4 mat = math::matrix_4x4::uninitialized;
  math::matrix_4x4 mrot = math::matrix_4x4::uninitialized;

  bool calc;
  void calcMatrix(Bone* allbones, int anim, int time, int animtime);
  Bone ( const MPQFile& f,
         const ModelBoneDef &b,
         int *global,
         const std::vector<std::unique_ptr<MPQFile>>& animation_files
       );

};


class TextureAnim {
  Animation::M2Value<math::vector_3d> trans, rot, scale;

public:
  math::vector_3d tval, rval, sval;

  void calc(int anim, int time, int animtime);
  TextureAnim(const MPQFile& f, const ModelTexAnimDef &mta, int *global);
  void setup(int anim);
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

// copied from the .mdl docs? this might be completely wrong
enum BlendModes {
  BM_OPAQUE,
  BM_TRANSPARENT,
  BM_ALPHA_BLEND,
  BM_ADDITIVE,
  BM_ADDITIVE_ALPHA,
  BM_MODULATE,
  BM_MODULATE2
};

#define MODEL_PIXEL_SHADER_COUNT 35

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
  Invalid_Shader,
};


struct ModelRenderPass : ModelTexUnit
{
  ModelRenderPass(ModelTexUnit const& tex_unit, Model* m);

  float ordering_thingy = 0.f;
  uint16_t index_start = 0, index_count = 0, vertex_start = 0, vertex_end = 0;
  uint16_t blend_mode = 0;
  ModelPixelShader pixel_shader = ModelPixelShader::Combiners_Opaque;

  bool prepare_draw(opengl::scoped::use_program& m2_shader, Model *m);
  void after_draw();
  void bind_texture(size_t index, Model* m);

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

struct ModelCamera {

  math::vector_3d pos, target;
  float nearclip, farclip, fov;
  Animation::M2Value<math::vector_3d> tPos, tTarget;
  Animation::M2Value<float> rot;

  ModelCamera(const MPQFile& f, const ModelCameraDef &mcd, int *global);
  void setup (float aspect_ratio, int time, int animtime);
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

struct model_vertex
{
  ::math::vector_3d position;
  ::math::vector_3d normal;
  ::math::vector_2d texcoords[2];
};

struct model_vertex_parameter
{
  uint8_t weights[4];
  uint8_t bones[4];
};

class Model : public AsyncObject
{
public:
   Model(const std::string& name);
  ~Model();

  void draw (bool draw_fog, int animtime, bool draw_particles);
  void draw ( std::vector<ModelInstance*> instances
            , opengl::scoped::use_program& m2_shader
            , math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool draw_fog
            , int animtime
            , bool draw_particles
            , bool all_boxes
            , std::unordered_map<Model*, std::size_t>& visible_model_count
            );
  void draw_box (opengl::scoped::use_program& m2_box_shader, std::size_t box_count);
  void drawTileMode();

  std::vector<float> intersect (math::ray const&, int animtime);

  void updateEmitters(float dt);

  virtual void finishLoading();

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
  std::vector<uint16_t> _texture_unit_lookup;

  // ===============================
  // Misc ?
  // ===============================
  std::string _filename; //! \todo ManagedItem already has a name. Use that?
  boost::optional<ModelCamera> cam;
  std::vector<Bone> bones;
  ModelHeader header;
  std::vector<uint16_t> blend_override;

  float rad;
  float trans;
  bool animcalc;
  bool mPerInstanceAnimation;
  int anim, animtime;
  int _global_animtime;

private:
  void initCommon(const MPQFile& f);
  bool isAnimated(const MPQFile& f);
  void initAnimated(const MPQFile& f);

  void fix_shader_id_blend_override();
  void fix_shader_id_layer();
  void compute_pixel_shader_ids();

  void animate(int anim, int animtime);
  void calcBones(int anim, int time, int animtime);

  void lightsOn(opengl::light lbase);
  void lightsOff(opengl::light lbase);

  void upload();

  bool _finished_upload;

  std::vector<math::vector_3d> _vertex_box_points;

  // ===============================
  // Geometry
  // ===============================
  GLuint _vao;
  GLuint _transform_buffer;
  GLuint _vertices_buffer;

  GLuint _box_vao;
  GLuint _box_vbo;

  std::vector<model_vertex> _vertices;
  std::vector<model_vertex> _current_vertices;

  std::vector<uint16_t> _indices;

  std::vector<model_vertex_parameter> _vertices_parameters;

  //std::vector<ModelRenderPass> _passes;
  std::vector<ModelRenderPass> _render_passes;

  // ===============================
  // Animation
  // ===============================
  bool animated;
  bool animGeometry, animTextures, animBones;

  std::vector<ModelRenderFlags> _render_flags;
  std::vector<ParticleSystem> _particles;
  std::vector<RibbonEmitter> _ribbons;

  std::vector<ModelAnimation> _animations;
  std::vector<int> _global_sequences;
  std::vector<TextureAnim> _texture_animations;
  std::vector<uint16_t> _texture_lookup;

  // ===============================
  // Material
  // ===============================
  std::vector<ModelColor> _colors;
  std::vector<ModelTransparency> _transparency;
  std::vector<int16_t> _transparency_lookup;
  std::vector<ModelLight> _lights;

  friend struct ModelRenderPass;
};

