#ifndef MODELHEADERS_H
#define MODELHEADERS_H

#include <stdint.h>

#include "vec3d.h"

#pragma pack(push,1)

struct Vertex {
	float tu, tv;
	float x, y, z;
};

struct ModelHeader {
	char id[4];
	uint8_t version[4];
	uint32_t nameLength;
	uint32_t nameOfs;
	uint32_t Flags;

	uint32_t nGlobalSequences;
	uint32_t ofsGlobalSequences;
	uint32_t nAnimations;
	uint32_t ofsAnimations;
	uint32_t nAnimationLookup;
	uint32_t ofsAnimationLookup;
	uint32_t nBones;
	uint32_t ofsBones;
	uint32_t nKeyBoneLookup;
	uint32_t ofsKeyBoneLookup;

	uint32_t nVertices;
	uint32_t ofsVertices;
	uint32_t nViews;

	uint32_t nColors;
	uint32_t ofsColors;

	uint32_t nTextures;
	uint32_t ofsTextures;

	uint32_t nTransparency; // H
	uint32_t ofsTransparency;
	uint32_t nTexAnims; // J
	uint32_t ofsTexAnims;
	uint32_t nTexReplace;
	uint32_t ofsTexReplace;

	uint32_t nRenderFlags;
	uint32_t ofsRenderFlags;
	uint32_t nBoneLookupTable;
	uint32_t ofsBoneLookupTable;

	uint32_t nTexLookup;
	uint32_t ofsTexLookup;

	uint32_t nTexUnitLookup; // L
	uint32_t ofsTexUnitLookup;
	uint32_t nTransparencyLookup; // M
	uint32_t ofsTransparencyLookup;
	uint32_t nTexAnimLookup;
	uint32_t ofsTexAnimLookup;

	//not sure about these :/
	Vec3D VertexBoxMin;//?
	Vec3D VertexBoxMax;//?
	float VertexBoxRadius;
	Vec3D BoundingBoxMin;//?
	Vec3D BoundingBoxMax;//?
	float BoundingBoxRadius;

	uint32_t nBoundingTriangles;
	uint32_t ofsBoundingTriangles;
	uint32_t nBoundingVertices;
	uint32_t ofsBoundingVertices;
	uint32_t nBoundingNormals;
	uint32_t ofsBoundingNormals;

	uint32_t nAttachments; // O
	uint32_t ofsAttachments;
	uint32_t nAttachLookup; // P
	uint32_t ofsAttachLookup;
	uint32_t nEvents; // SoundEvents
	uint32_t ofsEvents;
	uint32_t nLights; // R
	uint32_t ofsLights;
	uint32_t nCameras; // S
	uint32_t ofsCameras;
	uint32_t nCameraLookup;
	uint32_t ofsCameraLookup;
	uint32_t nRibbonEmitters; // U
	uint32_t ofsRibbonEmitters;
	uint32_t nParticleEmitters; // V
	uint32_t ofsParticleEmitters;

};


//only available if header->flags &0x8
//something about shadingflags
struct ofsUnk{
	int nUnk;
	int ofsnk;
};

struct ModelAnimation {
	int16_t animID;
	int16_t subAnimID;
	uint32_t length;

	float moveSpeed;

	uint32_t loopType;
	uint32_t flags;
	uint32_t d1;
	uint32_t d2;
	uint32_t playSpeed; // note: this can't be play speed because it's 0 for some models

	Vec3D boxA, boxB;
	float rad;

	int16_t NextAnimation;
	int16_t Index;
};

struct AnimationBlock {
	int16_t type; // interpolation type (0=none, 1=linear, 2=hermite, 3=Bezier)
	int16_t seq; // global sequence id or -1
	uint32_t nTimes;
	uint32_t ofsTimes;
	uint32_t nKeys;
	uint32_t ofsKeys;
};

struct FakeAnimationBlock {
	uint32_t nTimes;
	uint32_t ofsTimes;
	uint32_t nKeys;
	uint32_t ofsKeys;
};

struct AnimationBlockHeader {
	uint32_t nEntrys;
	uint32_t ofsEntrys;
};

struct AnimSubStructure {
	uint32_t n;
	uint32_t ofs;
};

struct ModelTexAnimDef {
	AnimationBlock trans, rot, scale;
};

struct ModelVertex {
	Vec3D pos;
	uint8_t weights[4];
	uint8_t bones[4];
	Vec3D normal;
	Vec2D texcoords;
	int unk1, unk2; // always 0,0 so this is probably unused
};

struct ModelView {
	char id[4]; // Signature
	uint32_t nIndex, ofsIndex; // Vertices in this model (index into vertices[])
	uint32_t nTris, ofsTris; // indices
	uint32_t nProps, ofsProps; // additional vtx properties
	uint32_t nSub, ofsSub; // materials/renderops/submeshes
	uint32_t nTex, ofsTex; // material properties/textures
	int32_t lod; // LOD bones
};

/// One material + render operation
struct ModelGeoset {
	uint16_t id; // mesh part id?
	uint16_t d2; // ?
	uint16_t vstart; // first vertex
	uint16_t vcount; // num vertices
	uint16_t istart; // first index
	uint16_t icount; // num indices
	uint16_t d3; // number of bone indices
	uint16_t d4; // ? always 1 to 4
	uint16_t d5; // ?
	uint16_t d6; // root bone?
	Vec3D v;
	float unknown[4]; // Added in WoW 2.0?
};

/// A texture unit (sub of material)
struct ModelTexUnit {
	// probably the texture units
	// size always >=number of materials it seems
	uint16_t flags; // Flags
	uint16_t order; // ?
	uint16_t op; // Material this texture is part of (index into mat)
	uint16_t op2; // Always same as above?
	int16_t colorIndex; // color or -1
	uint16_t flagsIndex; // more flags...
	uint16_t texunit; // Texture unit (0 or 1)
	uint16_t d4; // ? (seems to be always 1)
	uint16_t textureid; // Texture id (index into global texture list)
	uint16_t texunit2; // copy of texture unit value?
	uint16_t transid; // transparency id (index into transparency list)
	uint16_t texanimid; // texture animation id
};

// block X - render flags
struct ModelRenderFlags {
	uint16_t flags;
	//unsigned char f1;
	//unsigned char f2;
	uint16_t blend;
};

// block G - color defs
struct ModelColorDef {
	AnimationBlock color;
	AnimationBlock opacity;
};

// block H - transp defs
struct ModelTransDef {
	AnimationBlock trans;
};

struct ModelTextureDef {
	uint32_t type;
	uint32_t flags;
	uint32_t nameLen;
	uint32_t nameOfs;
};
struct ModelLightDef {
	int16_t type;
	int16_t bone;
	Vec3D pos;
	AnimationBlock ambColor;
	AnimationBlock ambIntensity;
	AnimationBlock diffcolor;
	AnimationBlock diffintensity;
	AnimationBlock attStart;
	AnimationBlock attEnd;
	AnimationBlock Enabled;
};

struct ModelCameraDef {
	int32_t id;
	float fov, farclip, nearclip;
	AnimationBlock transPos;
	Vec3D pos;
	AnimationBlock transTarget;
	Vec3D target;
	AnimationBlock rot;
};

struct ModelParticleEmitterDef {
	/// TODO: Do this one right. ._.

	int32_t id;
	int16_t flags;
	int16_t flags2;
	Vec3D pos;
	int16_t bone;
	int16_t texture;
	int32_t lenModelName;
	int32_t ofsModelName;
	int32_t lenParticleName;
	int32_t ofsParticleName;
	int8_t blend;
	int16_t emittertype;
	int16_t particletype;
	int8_t pad;
	int16_t rot;
	int16_t rows;
	int16_t cols;
	AnimationBlock Emissionspeed;
	AnimationBlock SpeedVariation;
	AnimationBlock VerticalRange;
	AnimationBlock HorizontalRange;
	AnimationBlock Gravity;
	AnimationBlock Lifespan;
	int32_t unknown1;
	AnimationBlock EmissionRate;
	int32_t unknown2;
	AnimationBlock EmissionAreaLength;
	AnimationBlock EmissionAreaWidth;
	AnimationBlock Gravity2;
	FakeAnimationBlock colors;
	FakeAnimationBlock opacity;
	FakeAnimationBlock sizes;
	int32_t d[2];
	FakeAnimationBlock intensity;
	FakeAnimationBlock unknownblock;
	float unk[3];
	float scales[3];
	float slowdown;
	float unknown3[2];
	float rotation; //Sprite Rotation
	float unknown4[2];
	float Rot1[3]; //Model Rotation 1
	float Rot2[3]; //Model Rotation 2
	float Trans[3]; //Model Translation
	float f2[4];
	int nUnknownReference;
	int ofsUnknownReference;
	AnimationBlock Enabled;
	AnimationBlock p;			// temporary to get rid of errors.
};

struct ModelRibbonEmitterDef {
	int32_t id;
	int32_t bone;
	Vec3D pos;
	int32_t nTextures;
	int32_t ofsTextures;
	int32_t nUnknown;
	int32_t ofsUnknown;
	AnimationBlock color;
	AnimationBlock opacity;
	AnimationBlock above;
	AnimationBlock below;
	float res, length, unk;
	int16_t s1, s2;
	AnimationBlock unk1;
	AnimationBlock enabled;
	int32_t unknown;
};

struct ModelEvents {
	char id[4];
	int32_t data;
	int32_t bone;
	Vec3D pos;
	int16_t type;
	int16_t seq;
	uint32_t nTimes;
	uint32_t ofsTimes;
};

struct ModelAttachmentDef {
	int32_t id;
	int32_t bone;
	Vec3D pos;
	AnimationBlock Enabled;
};

struct ModelBoneDef {
	int32_t KeyBoneID;
	uint32_t flags;
	int16_t parent; // parent bone index
	uint16_t unk[3];
	AnimationBlock translation;
	AnimationBlock rotation;
	AnimationBlock scaling;
	Vec3D pivot;
};

struct ModelBoundTriangle {
	uint16_t index[3];
};

#pragma pack(pop)


#endif
