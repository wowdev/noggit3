// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <vector>
#include <string>

class Model;
class Bone;

#include <opengl/types.h>

#include <math/matrix_4x4.h>
#include <math/vector_3d.h>
#include <math/vector_4d.h>
#include <math/ray.hpp>

#include <noggit/async/object.h>
#include <noggit/Animated.h> // Animation::M2Value
#include <noggit/ModelHeaders.h>
#include <noggit/TextureManager.h>
#include <noggit/Selection.h>

#include <boost/optional.hpp>

namespace noggit
{
  class blp_texture;
  namespace mpq
  {
    class file;
  }
}

::math::vector_3d fixCoordSystem(const ::math::vector_3d& v);
math::vector_3d convert_rotation (math::vector_3d const&);
math::quaternion convert_rotation (math::quaternion const&);

class Bone {
  Animation::M2Value< ::math::vector_3d> trans;
  Animation::M2Value< ::math::quaternion, ::math::packed_quaternion> rot;
  Animation::M2Value< ::math::vector_3d> scale;

public:
  ::math::vector_3d pivot;
  ::math::vector_3d transPivot;
  int parent;

  bool billboard;
  ::math::matrix_4x4 mat;
  ::math::matrix_4x4 mrot;

  bool calc;
  void calcMatrix(Bone* allbones, int anim, int time);
  Bone (const noggit::mpq::file& f, const ModelBoneDef &b, int *global, noggit::mpq::file **animfiles);
};


class TextureAnim {
  Animation::M2Value< ::math::vector_3d> trans, rot, scale;

public:
  ::math::vector_3d tval, rval, sval;

  void calc(int anim, int time);
  TextureAnim (const noggit::mpq::file& f, const ModelTexAnimDef &mta, int *global);
  void setup(int anim);
};

struct ModelColor {
  Animation::M2Value< ::math::vector_3d> color;
  Animation::M2Value<float,int16_t> opacity;

  ModelColor (const noggit::mpq::file& f, const ModelColorDef &mcd, int *global);
};

struct ModelTransparency {
  Animation::M2Value<float,int16_t> trans;

  ModelTransparency (const noggit::mpq::file& f, const ModelTransDef &mtd, int *global);
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

struct ModelRenderPass {
  uint16_t indexStart, indexCount, vertexStart, vertexEnd;
  int tex;
  bool usetex2, useenvmap, cull, trans, unlit, nozwrite, billboard;
  float p;

  int16_t texanim, color, opacity, blendmode, order;

  // Geoset ID
  int geoset;

  // texture wrapping
  bool swrap, twrap;

  // colours
  ::math::vector_4d ocol;
  ::math::vector_4d ecol;

  bool init(Model *m, int animtime);
  void deinit();

  bool operator< (const ModelRenderPass &m) const
  {
    //return !trans;
    if (order<m.order) return true;
    else if (order>m.order) return false;
    else return blendmode == m.blendmode ? (p<m.p) : blendmode < m.blendmode;
  }
};

struct ModelCamera {
  ::math::vector_3d pos;
  ::math::vector_3d target;
  float nearclip;
  float farclip;
  float fov;
  Animation::M2Value< ::math::vector_3d> tPos;
  Animation::M2Value< ::math::vector_3d> tTarget;
  Animation::M2Value<float> rot;

  ModelCamera(const noggit::mpq::file& f, const ModelCameraDef &mcd, int *global);
  void setup(int time=0);
};

struct ModelLight {
  int type;
  int parent;
  ::math::vector_3d pos;
  ::math::vector_3d tpos;
  ::math::vector_3d dir;
  ::math::vector_3d tdir;
  Animation::M2Value< ::math::vector_3d> diffColor;
  Animation::M2Value< ::math::vector_3d> ambColor;
  Animation::M2Value<float> diffIntensity;
  Animation::M2Value<float> ambIntensity;
  //Animation::M2Value<float> attStart,attEnd;
  //Animation::M2Value<bool> Enabled;

  ModelLight (const noggit::mpq::file&  f, const ModelLightDef &mld, int *global);
  void setup(int time, opengl::light l);
};

struct model_vertex
{
  ::math::vector_3d position;
  ::math::vector_3d normal;
  ::math::vector_2d texcoords;
};

struct model_vertex_parameter
{
  uint8_t weights[4];
  uint8_t bones[4];
};

class Model: public noggit::async::object
{
public:
  Model(const std::string& name, bool forceAnim=false);
  ~Model();

  void draw (bool draw_fog, size_t time);
  void drawTileMode();

  void intersect (size_t time, math::ray ray, std::vector<float>& results);

  void updateEmitters(float dt);

  virtual void finish_loading();



  //! \todo ManagedItem already has a name. Use that?
  std::string _filename;
  ModelHeader header;

  float rad;
  float trans;
  bool animcalc;

private:
  void drawModel (int animtime);

  void initCommon (const noggit::mpq::file& f);
  bool isAnimated (const noggit::mpq::file& f);
  void initAnimated (const noggit::mpq::file& f);

  void animate (int anim, int time);
  void calcBones (int anim, int time);

  void lightsOn (opengl::light lbase, int animtime);
  void lightsOff (opengl::light lbase);

  void upload ();



  bool _has_animation;
  bool _has_geometry_animation;
  bool _has_texture_animation;
  bool _has_bone_animation;

  bool _per_instance_animation;
  bool _force_animation;

  int _current_animation;

  bool _finished_upload;

  boost::optional<ModelCamera> _camera;

  GLuint _vertices_buffer;

  std::vector<model_vertex> _vertices;
  std::vector<model_vertex> _current_vertices;

  std::vector<uint16_t> _indices;

  std::vector<model_vertex_parameter> _vertices_parameters;

  std::vector<std::string> _texture_names;
  std::vector<noggit::scoped_blp_texture_reference> _textures;

  std::vector<noggit::scoped_blp_texture_reference> _replaceTextures;
  std::vector<bool> _useReplaceTextures;

  std::vector<ModelRenderPass> _passes;
  std::vector<ModelAnimation> _animations;
  std::vector<int> _global_sequences;
  std::vector<TextureAnim> _texture_animations;
  std::vector<ModelColor> _colors;
  std::vector<ModelTransparency> _transparency;
  std::vector<ModelLight> _lights;
  std::vector<Bone> _bones;

  friend struct ModelRenderPass;
};
