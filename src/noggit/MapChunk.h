#ifndef MAPCHUNK_H
#define MAPCHUNK_H

#include <noggit/MapTile.h> // MapTile
#include <noggit/Quaternion.h> // Vec4D

namespace noggit
{
  class blp_texture;
  namespace mpq
  {
    class file;
  }
}
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
  noggit::blp_texture* _textures[4];
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

  MapChunk(World* world, MapTile* mt, noggit::mpq::file* f,bool bigAlpha);
  ~MapChunk();

  void destroy();
  void initStrip();

  void draw ( bool draw_terrain_height_contour
            , bool mark_impassable_chunks
            , bool draw_area_id_overlay
            , bool dont_draw_cursor
            );
  void drawContour();
  void drawAreaID();
  void drawBlock();
  void drawColor (bool draw_fog);
  void drawSelect();
  void drawNoDetail();
  void drawPass(int anim, int animation_time = 0);
  // todo split into draw_lines and draw_hole_lines
  void drawLines (bool draw_hole_lines);

  void drawTextures (int animation_time);

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

  bool paintTexture(float x, float z, brush *Brush, float strength, float pressure, noggit::blp_texture* texture);
  int addTexture(noggit::blp_texture* texture);
  void switchTexture( noggit::blp_texture* oldTexture, noggit::blp_texture* newTexture );
  void eraseTextures();

  bool isHole(int i,int j);
  void addHole(int i,int j);
  void removeHole(int i,int j);

  void setFlag(bool on_or_off, int flag);

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x,float z, Vec3D *V);

  void loadTextures();
//  char getAlpha(float x,float y);


  //float getTerrainHeight(float x, float z);

  void SetAnim (int anim, const float& anim_time) const;
  void RemoveAnim (int anim) const;

  void GenerateContourMap();
  void CreateStrips();

private:
  World* _world;

  GLuint _contour_texture;
  float _contour_coord_gen[4];

  StripType _odd_strips[8*18];
  StripType _even_strips[8*18];
  StripType _line_strip[32];
  StripType _hole_strip[128];
};

#endif // MAPCHUNK_H
