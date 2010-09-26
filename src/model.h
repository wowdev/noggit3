#ifndef MODEL_H
#define MODEL_H

#include <vector>

class Model;
class Bone;

#include "mpq.h"

#include "animated.h"
#include "particle.h"

#include "quaternion.h"
#include "matrix.h"
#include "vec3d.h"

#include "modelheaders.h"

#include "video.h" // GLuint
#include "manager.h" // ManagedItem
#include "AsyncObject.h" // AsyncObject

Vec3D fixCoordSystem(Vec3D v);

class Bone {
	

	Animated<Vec3D> trans;
	Animated<Quaternion> rot;
	Animated<Vec3D> scale;

public:
	Vec3D pivot;
	int parent;

	bool billboard;
	Matrix mat;
	Matrix mrot;

	bool calc;
	void calcMatrix(Bone* allbones, int anim, int time);
	void init(MPQFile &f, ModelBoneDef &b, int *global);

};


class TextureAnim {
	Animated<Vec3D> trans, rot, scale;

public:
	Vec3D tval, rval, sval;

	void calc(int anim, int time);
	void init(MPQFile &f, ModelTexAnimDef &mta, int *global);
	void setup();
};

struct ModelColor {
	Animated<Vec3D> color;
	AnimatedShort opacity;

	void init(MPQFile &f, ModelColorDef &mcd, int *global);
};

struct ModelTransparency {
	AnimatedShort trans;

	void init(MPQFile &f, ModelTransDef &mtd, int *global);
};

// copied from the .mdl docs? this might be completely wrong
enum BlendModes {
	BM_OPAQUE,
	BM_TRANSPARENT,
	BM_ALPHA_BLEND,
	BM_ADDITIVE,
	BM_ADDITIVE_ALPHA,
	BM_MODULATE
};

struct ModelRenderPass {
	uint16_t indexStart, indexCount, vertexStart, vertexEnd;
	GLuint texture, texture2;
	bool usetex2, useenvmap, cull, trans, unlit, nozwrite;
	float p;
	
	short texanim, color, opacity, blendmode, order;

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

	void init(MPQFile &f, ModelCameraDef &mcd, int *global);
	void setup(int time=0);

	ModelCamera():ok(false) {}
};

struct ModelLight {
	int type, parent;
	Vec3D pos, tpos, dir, tdir;
	Animated<Vec3D> diffColor, ambColor;
	Animated<float> diffIntensity, ambIntensity;
	Animated<float> attStart,attEnd;
	Animated<bool> Enabled;

	void init(MPQFile &f, ModelLightDef &mld, int *global);
	void setup(int time, GLuint l);
};

class Model: public ManagedItem, public AsyncObject {

	GLuint ModelDrawList;
	GLuint SelectModelDrawList;
	GLuint TileModeModelDrawList;

	GLuint vbuf, nbuf, tbuf;
	size_t vbufsize;
	bool animated;
	bool animGeometry,animTextures,animBones;
	bool forceAnim;

	void init(MPQFile &f);


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
	
	void initCommon(MPQFile &f);
	bool isAnimated(MPQFile &f);
	void initAnimated(MPQFile &f);
	void initStatic(MPQFile &f);

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
	std::string filename;
	ModelCamera cam;
	Bone *bones;
	GLuint *textures;
	ModelHeader header;

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
