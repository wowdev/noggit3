#ifndef MAPCHUNK_H
#define MAPCHUNK_H

#include "MapTile.h" // MapTile
#include "Quaternion.h" // Vec4D
#include "Video.h" // GLuint
#include "WMOInstance.h"
#include "ModelInstance.h"

class MPQFile;
class Vec4D;
class Brush;
class Alphamap;
class TextureSet;
class sExtendableArray;

typedef unsigned short StripType;
static const int mapbufsize = 9*9 + 8*8;

class MapChunk
{
//! ------ Variables ---------
public:
  //! \todo make most of these private

  MapTile *mt;
  Vec3D vmin, vmax, vcenter;
  int px, py;

  MapChunkHeader header;
  bool Changed;

  float xbase, ybase, zbase;

  unsigned int Flags;

  TextureSet* textureSet;

  GLuint vertices, normals, minimap, minishadows;

  Vec3D mVertices[mapbufsize];

private:
  float r;

  bool mBigAlpha;
  bool haswater;

  int nameID;
  int holes;

  unsigned int areaID;

  unsigned char mShadowMap[8*64];
  GLuint shadow;

  StripType *strip;
  int striplen;

  Vec3D mNormals[mapbufsize];
  //! \todo Is this needed? Can't we just use the real vertices?
  Vec3D mMinimap[mapbufsize];
  Vec4D mFakeShadows[mapbufsize];

//! ------ Functions ---------
public:
  MapChunk(MapTile* mt, MPQFile* f,bool bigAlpha);
  ~MapChunk();

  void draw(); //! \todo only this function should be public, all others should be called from it

  void drawContour();
  void drawSelect();
  void drawLines();
  void drawTextures();

  void recalcNorms();

  void getSelectionCoord(float *x,float *z);
  float getSelectionHeight();

  Vec3D GetSelectionPosition();

  //! \todo implement Action stack for these
  bool changeTerrain(float x, float z, float change, float radius, int BrushType);
  bool flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  bool blurTerrain(float x, float z, float remain, float radius, int BrushType);

  //! \todo implement Action stack for these
  bool paintTexture(float x, float z, Brush *brush, float strength, float pressure, OpenGL::Texture* texture);
  int addTexture(OpenGL::Texture* texture);
  void switchTexture( OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture );
  void eraseTextures();

  //! \todo implement Action stack for these
  bool isHole(int i,int j);
  void addHole(int i,int j);
  void addHoleBig(int i, int j);
  void removeHole(int i,int j);
  void removeHoleBig(int i,int j);

  void setFlag(bool value);

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x,float z, Vec3D *V);

  //! \todo this is ugly create a build struct or sth
  void save(sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::map<int, WMOInstance> &lObjectInstances, std::map<int, ModelInstance> &lModelInstances);

  //char getAlpha(float x,float y);
  //float getTerrainHeight(float x, float z);

  //void destroy(); wtf? does not exist
  //void drawAreaID(); wtf? does not exist
  //void drawBlock(); wtf? does not exist
  //void loadTextures(); not used
  //void drawColor(); not used
  //void drawNoDetail(); not used

private:
  void initStrip();

  int indexNoLoD(int x, int y);
  int indexLoD(int x, int y);

  void drawPass(int id);

};

#endif // MAPCHUNK_H
