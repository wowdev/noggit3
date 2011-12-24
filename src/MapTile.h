#ifndef MAPTILE_H
#define MAPTILE_H

#include <vector>
#include <map>
#include <string>

#include "MapHeaders.h"
#include "Video.h" // GLfloat, GLshort, ...

class Vec3D;
class Liquid;
class World;
class MapChunk;

class MapTile
{
public:
  MapTile( World*, int x0, int z0, const std::string& pFilename, bool pBigAlpha );
  ~MapTile();

  //! \brief Get the maximum height of terrain on this map tile.
  float getMaxHeight();

  //! \brief Get chunk for sub offset x,z.
  MapChunk* getChunk( unsigned int x, unsigned int z );

  int modelCount;
  int mPositionX;
  int mPositionZ;
  float xbase, zbase;

  bool changed;

  void draw ( bool draw_terrain_height_contour
            , bool mark_impassable_chunks
            , bool draw_area_id_overlay
            , bool dont_draw_cursor
            );
  void drawSelect();
  void drawLines();
  void drawWater();
  void drawTextures (int animation_time);
  void drawMFBO();

  bool GetVertex( float x, float z, Vec3D *V );

  void saveTile();

  bool isTile( int pX, int pZ );
  void clearAllModels();

private:
  // MFBO:
  GLfloat mMinimumValues[3*3*3];
  GLfloat mMaximumValues[3*3*3];

  // MHDR:
  int mFlags;
  bool mBigAlpha;

  // Data to be loaded and later unloaded.
  std::vector<std::string> mTextureFilenames;
  std::vector<std::string> mModelFilenames;
  std::vector<std::string> mWMOFilenames;

  std::string mFilename;

  MapChunk * mChunks[16][16];
  std::vector<Liquid*> mLiquids;

  World* _world;

  friend class MapChunk;
};

#endif
