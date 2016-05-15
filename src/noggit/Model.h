// Model.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

class Model;
class Bone;

#include <opengl/types.h>

#include <math/matrix_4x4.h>
#include <math/vector_3d.h>
#include <math/vector_4d.h>

#include <noggit/async/object.h>
#include <noggit/Animated.h> // Animation::M2Value
#include <noggit/ModelHeaders.h>
#include <noggit/Particle.h>
#include <noggit/TextureManager.h>

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

class Bone {
  Animation::M2Value< ::math::vector_3d> trans;
  Animation::M2Value< ::math::quaternion, ::math::packed_quaternion> rot;
  Animation::M2Value< ::math::vector_3d> scale;

public:
  ::math::vector_3d pivot, transPivot;
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
  ::math::vector_3d pos, target;
  float nearclip, farclip, fov;
  Animation::M2Value< ::math::vector_3d> tPos, tTarget;
  Animation::M2Value<float> rot;

  ModelCamera(const noggit::mpq::file& f, const ModelCameraDef &mcd, int *global);
  void setup(int time=0);
};

struct ModelLight {
  int type, parent;
  ::math::vector_3d pos, tpos, dir, tdir;
  Animation::M2Value< ::math::vector_3d> diffColor, ambColor;
  Animation::M2Value<float> diffIntensity, ambIntensity;
  //Animation::M2Value<float> attStart,attEnd;
  //Animation::M2Value<bool> Enabled;

  ModelLight (const noggit::mpq::file&  f, const ModelLightDef &mld, int *global);
  void setup(int time, opengl::light l);
};

class Model: public noggit::async::object
{

  GLuint ModelDrawList;
  GLuint SelectModelDrawList;
  //GLuint TileModeModelDrawList;

  GLuint vbuf, nbuf, tbuf;
  size_t vbufsize;
  bool animated;
  bool animGeometry,animTextures,animBones;
  bool forceAnim;
  noggit::mpq::file **animfiles;

  std::vector<TextureAnim> texanims;
  ModelAnimation *anims;
  int *globalSequences;
  std::vector<ModelColor> colors;
  std::vector<ModelTransparency> transparency;
  std::vector<ModelLight> lights;
  std::vector<ParticleSystem> particleSystems;
  std::vector<RibbonEmitter> ribbons;

  void drawModel (int animtime);
  void drawModelSelect (int animtime);

  void initCommon(const noggit::mpq::file& f);
  bool isAnimated(const noggit::mpq::file& f);
  void initAnimated(const noggit::mpq::file& f);
  void initStatic(const noggit::mpq::file& f);

  ModelVertex *origVertices;
  ::math::vector_3d *vertices, *normals;
  uint16_t *indices;
  size_t nIndices;
  std::vector<ModelRenderPass> passes;

  void animate(int anim, int time);
  void calcBones(int anim, int time);

  void lightsOn(opengl::light lbase, int animtime);
  void lightsOff(opengl::light lbase);

public:
  std::string _filename; //! \todo ManagedItem already has a name. Use that?
  boost::optional<ModelCamera> cam;
  std::vector<Bone> bones;
  ModelHeader header;

  // ===============================
  // Toggles
  bool *showGeosets;

  // ===============================
  // Texture data
  // ===============================
  std::vector<noggit::scoped_blp_texture_reference> _textures;
  std::vector<noggit::scoped_blp_texture_reference> _replaceTextures;
  std::vector<bool> _useReplaceTextures;

  float rad;
  float trans;
  bool animcalc;
  bool mPerInstanceAnimation;
  int anim;

  Model(const std::string& name, bool forceAnim=false);
  ~Model();
  void draw (bool draw_fog, size_t time);
  void drawTileMode();
  void drawSelect(size_t time);
  void updateEmitters(float dt);

  friend struct ModelRenderPass;

  virtual void finish_loading();
};

#endif
