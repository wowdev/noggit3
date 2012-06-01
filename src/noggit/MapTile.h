// MapTile.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MAPTILE_H
#define MAPTILE_H

#include <vector>
#include <map>
#include <string>

#include <opengl/types.h>

#include <noggit/MapHeaders.h>

class Liquid;
class World;
class MapChunk;

class QRectF;

namespace math
{
  class vector_3d;
}

class Skies;

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
            , const float& animtime
            );
  void drawSelect (const float& culldistance);
  void drawLines (bool draw_hole_lines);
  void drawWater (const float& animtime, const Skies* skies);
  void drawTextures ( const QRectF& chunks_to_draw
                    , int animation_time
                    ) const;
  void drawMFBO();

  bool GetVertex( float x, float z, ::math::vector_3d *V );

  void saveTile();
  void saveTileCata();

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
