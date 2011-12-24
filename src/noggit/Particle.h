#ifndef PARTICLE_H
#define PARTICLE_H

#include <list>
#include <vector>

class ParticleSystem;
class RibbonEmitter;

#include <noggit/Animated.h> // Animation::M2Value
#include <noggit/Model.h>
#include <noggit/Video.h>

struct Particle {
  Vec3D pos, speed, down, origin, dir;
  Vec3D  corners[4];
  //Vec3D tpos;
  float size, life, maxlife;
  unsigned int tile;
  Vec4D color;
};

typedef std::list<Particle> ParticleList;

class ParticleEmitter {
protected:
  ParticleSystem *sys;
public:
  explicit ParticleEmitter(ParticleSystem *psys): sys(psys) {}
  virtual ~ParticleEmitter() {}
  virtual Particle newParticle(int anim, int time, float w, float l, float spd, float var, float spr, float spr2) = 0;
};

class PlaneParticleEmitter: public ParticleEmitter {
public:
  explicit PlaneParticleEmitter(ParticleSystem *_sys): ParticleEmitter(_sys) {}
  virtual ~PlaneParticleEmitter() {}
  Particle newParticle(int anim, int time, float w, float l, float spd, float var, float spr, float spr2);
};

class SphereParticleEmitter: public ParticleEmitter {
public:
  explicit SphereParticleEmitter(ParticleSystem *_sys): ParticleEmitter(_sys) {}
  virtual ~SphereParticleEmitter() {}
  Particle newParticle(int anim, int time, float w, float l, float spd, float var, float spr, float spr2);
};

struct TexCoordSet {
    Vec2D tc[4];
};

class ParticleSystem {
  Animation::M2Value<float> speed, variation, spread, lat, gravity, lifespan, rate, areal, areaw, deacceleration;
  Animation::M2Value<uint8_t> enabled;
  Vec4D colors[3];
  float sizes[3];
  ParticleEmitter *emitter;
  float mid, slowdown, rotation;
  Vec3D pos;
  OpenGL::Texture* _texture;
  ParticleList particles;
  int blend,order,type;
  int manim,mtime;
  int rows, cols;
  std::vector<TexCoordSet> tiles;
  void initTile(Vec2D *tc, int num);
  bool billboard;

  float rem;
  //bool transform;

  // unknown parameters omitted for now ...
  Bone *parent;
  int32_t flags;
  int16_t pType;

public:
  Model *model;
  float tofs;

  ParticleSystem(): emitter(NULL), mid(0), rem(0)
  {
    blend = 0;
    order = 0;
    type = 0;
    manim = 0;
    mtime = 0;
    rows = 0;
    cols = 0;

    model = 0;
    parent = 0;
    _texture = NULL;

    slowdown = 0;
    rotation = 0;
    tofs = 0;
  }
  virtual ~ParticleSystem() { if( emitter ) { delete emitter; emitter = NULL; } }

  void init(const MPQFile& f, const ModelParticleEmitterDef &mta, int *globals);
  void update(float dt);

  void setup(int anim, int time);
  void draw();
  void drawHighlight();

  friend class PlaneParticleEmitter;
  friend class SphereParticleEmitter;
};


struct RibbonSegment {
  Vec3D pos, up, back;
  float len,len0;
};

class RibbonEmitter {
  Animation::M2Value<Vec3D> color;
  Animation::M2Value<float,int16_t> opacity;
  Animation::M2Value<float> above, below;

  Bone *parent;
  float f1, f2;

  Vec3D pos;

  int manim, mtime;
  float length, seglen;
  int numsegs;

  Vec3D tpos;
  Vec4D tcolor;
  float tabove, tbelow;

  OpenGL::Texture* _texture;

  std::list<RibbonSegment> segs;

public:
  Model *model;

  void init(const MPQFile &f, ModelRibbonEmitterDef &mta, int *globals);
  void setup(int anim, int time);
  void draw();
};



#endif
