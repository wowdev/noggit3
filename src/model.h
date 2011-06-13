#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

class Model;
class Bone;

#include "Animated.h"
#include "AsyncObject.h" // AsyncObject
#include "Manager.h" // ManagedItem
#include "Matrix.h"
#include "ModelHeaders.h"
#include "MPQ.h"
#include "Particle.h"
#include "Quaternion.h"
#include "Vec3D.h"
#include "Video.h" // GLuint

Vec3D fixCoordSystem(Vec3D v);

class Bone {
  Animated<Vec3D> trans;
  Animated<Quaternion, PACK_QUATERNION, Quat16ToQuat32> rot;
  Animated<Vec3D> scale;

public:
  Vec3D pivot, transPivot;
  int parent;

  bool billboard;
  Matrix mat;
  Matrix mrot;

  bool calc;
  void calcMatrix(Bone* allbones, int anim, int time);
  void init(const MPQFile& f, const ModelBoneDef &b, int *global, MPQFile **animfiles);

};


class TextureAnim {
  Animated<Vec3D> trans, rot, scale;

public:
  Vec3D tval, rval, sval;

  void calc(int anim, int time);
  void init(const MPQFile& f, const ModelTexAnimDef &mta, int *global);
  void setup(int anim);
};

struct ModelColor {
  Animated<Vec3D> color;
  AnimatedShort opacity;

  void init(const MPQFile& f, const ModelColorDef &mcd, int *global);
};

struct ModelTransparency {
  AnimatedShort trans;

  void init(const MPQFile& f, const ModelTransDef &mtd, int *global);
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
  //GLuint texture, texture2;
  int tex;
  bool usetex2, useenvmap, cull, trans, unlit, nozwrite, billboard;
  float p;
  
  int16_t texanim, color, opacity, blendmode, order;

  // Geoset ID
  int geoset;
  
  // texture wrapping
  bool swrap, twrap;
  
  // colours
  Vec4D ocol, ecol;
  
  bool init(Model *m);
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
  bool ok;

  Vec3D pos, target;
  float nearclip, farclip, fov;
  Animated<Vec3D> tPos, tTarget;
  Animated<float> rot;

  void init(const MPQFile& f, const ModelCameraDef &mcd, int *global);
  void setup(int time=0);

  ModelCamera():ok(false) {}
};

struct ModelLight {
  int type, parent;
  Vec3D pos, tpos, dir, tdir;
  Animated<Vec3D> diffColor, ambColor;
  Animated<float> diffIntensity, ambIntensity;
  //Animated<float> attStart,attEnd;
  //Animated<bool> Enabled;

  void init(const MPQFile&  f, const ModelLightDef &mld, int *global);
  void setup(int time, GLuint l);
};

class Model: public ManagedItem, public AsyncObject {

  GLuint ModelDrawList;
  GLuint SelectModelDrawList;
  //GLuint TileModeModelDrawList;

  GLuint vbuf, nbuf, tbuf;
  size_t vbufsize;
  bool animated;
  bool animGeometry,animTextures,animBones;
  bool forceAnim;
  MPQFile **animfiles;

  void init(const MPQFile& f);


  TextureAnim *texanims;
  ModelAnimation *anims;
  int *globalSequences;
  ModelColor *colors;
  ModelTransparency *transparency;
  ModelLight *lights;
  ParticleSystem *particleSystems;
  RibbonEmitter *ribbons;

  void drawModel( /*bool unlit*/ );
  void drawModelSelect();
  
  void initCommon(const MPQFile& f);
  bool isAnimated(const MPQFile& f);
  void initAnimated(const MPQFile& f);
  void initStatic(const MPQFile& f);

  ModelVertex *origVertices;
  Vec3D *vertices, *normals;
  uint16_t *indices;
  size_t nIndices;
  std::vector<ModelRenderPass> passes;

  void animate(int anim);
  void calcBones(int anim, int time);

  void lightsOn(GLuint lbase);
  void lightsOff(GLuint lbase);

public:
  std::string filename; //! \todo ManagedItem already has a name. Use that?
  ModelCamera cam;
  Bone *bones;
  ModelHeader header;
  
  // ===============================
  // Toggles
  bool *showGeosets;
  
  // ===============================
  // Texture data
  // ===============================
  GLuint *textures;
  #define  TEXTURE_MAX  100
  //! \todo vectors.
  int specialTextures[TEXTURE_MAX];
  GLuint replaceTextures[TEXTURE_MAX];
  bool useReplaceTextures[TEXTURE_MAX];

  float rad;
  float trans;
  bool animcalc;
  bool mPerInstanceAnimation;
  int anim, animtime;

  Model(const std::string& name, bool forceAnim=false);  
  ~Model();
  void draw();
  void drawTileMode();
  void drawSelect();
  void updateEmitters(float dt);

  friend struct ModelRenderPass;
  
  virtual void finishLoading();
};

#endif
