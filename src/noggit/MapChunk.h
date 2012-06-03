// MapChunk.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MAPCHUNK_H
#define MAPCHUNK_H

#include <math/vector_4d.h>

#include <noggit/MapTile.h> // MapTile

namespace noggit
{
  class blp_texture;
  namespace mpq
  {
    class file;
  }
}
class brush;
class Frustum;
class World;
class Skies;

typedef unsigned short StripType;

static const int mapbufsize = 9*9 + 8*8;

class MapChunk
{

public:
  MapTile *mt;
  ::math::vector_3d vmin, vmax, vcenter;
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

  bool is_visible ( const float& cull_distance
                  , const Frustum& frustum
                  , const ::math::vector_3d& camera
                  ) const;

  void draw (bool draw_terrain_height_contour
            , bool mark_impassable_chunks
            , bool draw_area_id_overlay
            , bool dont_draw_cursor
            , const Skies* skies);
  void drawContour();
  void drawAreaID();
  void drawBlock();
  void drawSelect();
  void drawNoDetail();
  void drawPass(int anim);
  // todo split into draw_lines and draw_hole_lines
  void drawLines (bool draw_hole_lines);

  void drawTextures();

  void recalcNorms();

  ::math::vector_3d mNormals[mapbufsize];
  ::math::vector_3d mVertices[mapbufsize];
  //! \todo Is this needed? Can't we just use the real vertices?
  ::math::vector_3d mMinimap[mapbufsize];
  ::math::vector_4d mFakeShadows[mapbufsize];

  void getSelectionCoord(float *x,float *z);
  float getSelectionHeight();

  ::math::vector_3d GetSelectionPosition();

  bool changeTerrain(float x, float z, float change, float radius, int BrushType);
  bool flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  bool blurTerrain(float x, float z, float remain, float radius, int BrushType);

  bool paintTexture(float x, float z, const brush& Brush, float strength, float pressure, noggit::blp_texture* texture);
  int addTexture(noggit::blp_texture* texture);
  void switchTexture( noggit::blp_texture* oldTexture, noggit::blp_texture* newTexture );
  void eraseTextures();

  bool isHole(int i,int j);
  void addHole(int i,int j);
  void removeHole(int i,int j);

  void setFlag(bool on_or_off, int flag);

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x,float z, ::math::vector_3d* V);

  void loadTextures();
//  char getAlpha(float x,float y);


  //float getTerrainHeight(float x, float z);

  void SetAnim (int anim) const;
  void RemoveAnim (int anim) const;

  void GenerateContourMap();
  void CreateStrips();

  void update_low_quality_texture_map();
  const unsigned char* low_quality_texture_map() const;

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
