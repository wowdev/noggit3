#ifndef MAPCHUNK_H
#define MAPCHUNK_H

#include "MapTile.h" // MapTile
#include "Quaternion.h" // Vec4D
#include "Video.h" // GLuint

class MPQFile;
class Vec4D;
class brush;
class World;

typedef unsigned short StripType;

static const int mapbufsize = 9*9 + 8*8;

class MapChunk
{

public:
  MapTile *mt;
  Vec3D vmin, vmax, vcenter;
  int px, py;

  MapChunkHeader header;
  bool Changed;
  size_t nTextures;

  float xbase, ybase, zbase;
  float r;

  bool mBigAlpha;

  int nameID;

  unsigned int Flags;

  unsigned int areaID;

  bool haswater;

  int holes;

  int tex[4];
  OpenGL::Texture* _textures[4];
  unsigned int texFlags[4];
  unsigned int effectID[4];
  unsigned int MCALoffset[4];
  unsigned char amap[3][64*64];
  unsigned char mShadowMap[8*64];
  GLuint alphamaps[3];
  GLuint shadow;

  int animated[4];

  GLuint vertices, normals, minimap, minishadows;

  StripType *strip;
  int striplen;

  MapChunk(World* world, MapTile* mt, MPQFile* f,bool bigAlpha);
  ~MapChunk();

  void destroy();
  void initStrip();

  void draw();
  void drawContour();
  void drawAreaID();
  void drawBlock();
  void drawColor();
  void drawSelect();
  void drawNoDetail();
  void drawPass(int anim);
  void drawLines();

  void drawTextures();

  void recalcNorms();

  Vec3D mNormals[mapbufsize];
  Vec3D mVertices[mapbufsize];
  //! \todo Is this needed? Can't we just use the real vertices?
  Vec3D mMinimap[mapbufsize];
  Vec4D mFakeShadows[mapbufsize];

  void getSelectionCoord(float *x,float *z);
  float getSelectionHeight();

  Vec3D GetSelectionPosition();

  bool changeTerrain(float x, float z, float change, float radius, int BrushType);
  bool flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  bool blurTerrain(float x, float z, float remain, float radius, int BrushType);

  bool paintTexture(float x, float z, brush *Brush, float strength, float pressure, OpenGL::Texture* texture);
  int addTexture(OpenGL::Texture* texture);
  void switchTexture( OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture );
  void eraseTextures();

  bool isHole(int i,int j);
  void addHole(int i,int j);
  void removeHole(int i,int j);

  void setFlag(bool value);

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x,float z, Vec3D *V);

  void loadTextures();
//  char getAlpha(float x,float y);


  //float getTerrainHeight(float x, float z);

  void SetAnim (int anim, const float& anim_time) const;
  void RemoveAnim (int anim) const;

private:
  World* _world;
};

#endif // MAPCHUNK_H
