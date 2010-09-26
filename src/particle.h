
#ifndef PARTICLE_H
#define PARTICLE_H

class ParticleSystem;
class RibbonEmitter;

#include "model.h"
#include "animated.h"
#include "video.h" // GLuint

#include <list>

struct Particle {
	Vec3D pos, speed, down, origin;
	//Vec3D tpos;
	float size, life, maxlife;
	int tile;
	Vec4D color;
};

typedef std::list<Particle> ParticleList;

class ParticleEmitter {
protected:
	ParticleSystem *sys;
public:
	ParticleEmitter(ParticleSystem *psys): sys(psys) {}
	virtual Particle newParticle(int anim, int time) = 0;
};

class PlaneParticleEmitter: public ParticleEmitter {
public:
	PlaneParticleEmitter(ParticleSystem *_sys): ParticleEmitter(_sys) {}
	Particle newParticle(int anim, int time);
};

class SphereParticleEmitter: public ParticleEmitter {
public:
	SphereParticleEmitter(ParticleSystem *_sys): ParticleEmitter(_sys) {}
	Particle newParticle(int anim, int time);
};

struct TexCoordSet {
    Vec2D tc[4];
};

class ParticleSystem {
	Animated<float> speed, variation, spread, lat, gravity, lifespan, rate, areal, areaw, grav2;
	Animated<Vec3D> color;
	Animated<Vec2D> size;
	Animated<float,short,ShortToFloat> opac;
	Vec4D colors[3];
	float sizes[3];
	float mid, slowdown, rotation;
	Vec3D pos;
	GLuint texture;
	ParticleEmitter *emitter;
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

public:
	Model *model;
	float tofs;

	ParticleSystem(): emitter(0) {};
	~ParticleSystem() { delete emitter; }

	void init(MPQFile &f, ModelParticleEmitterDef &mta, int *globals);
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
	Animated<Vec3D> color;
	AnimatedShort opacity;
	Animated<float> above, below;

	Bone *parent;
	float f1, f2;

	Vec3D pos;

	int manim, mtime;
	float length, seglen;
	int numsegs;
	
	Vec3D tpos;
	Vec4D tcolor;
	float tabove, tbelow;

	GLuint texture;

	std::list<RibbonSegment> segs;

public:
	Model *model;

	void init(MPQFile &f, ModelRibbonEmitterDef &mta, int *globals);
	void setup(int anim, int time);
	void draw();
};



#endif
