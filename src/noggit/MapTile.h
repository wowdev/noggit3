// MapTile.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MAPTILE_H
#define MAPTILE_H

#include <vector>
#include <map>
#include <string>

#include <boost/optional.hpp>

#include <opengl/types.h>

#include <noggit/MapHeaders.h>
#include <noggit/World.h> // instances types

class Frustum;
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

  //! \brief Get chunk for sub offset x,z.
  MapChunk* getChunk( unsigned int x, unsigned int z );

  int mPositionX;
  int mPositionZ;
  float xbase;
  float zbase;

  void draw ( bool draw_terrain_height_contour
            , bool mark_impassable_chunks
            , bool draw_area_id_overlay
            , bool dont_draw_cursor
            , const Skies* skies
            , const float& cull_distance
            , const Frustum& frustum
            , const ::math::vector_3d& camera
            , const boost::optional<selection_type>& selected_item
            );
  void drawSelect ( const float& cull_distance
                  , const Frustum& frustum
                  , const ::math::vector_3d& camera
                  );
  void drawLines ( bool draw_hole_lines
                 , const float& cull_distance
                 , const Frustum& frustum
                 , const ::math::vector_3d& camera
                 );
  void drawWater (const Skies* skies);
  void drawTextures (const QRectF& chunks_to_draw) const;
  void drawMFBO();

  boost::optional<float> get_height ( const float& x
                                    , const float& z
                                    ) const;

  void saveTile ( const World::model_instances_type::const_iterator& models_begin
                , const World::model_instances_type::const_iterator& models_end
                , const World::wmo_instances_type::const_iterator& wmos_begin
                , const World::wmo_instances_type::const_iterator& wmos_end
                );
  void saveTileCata ( const World::model_instances_type::const_iterator& models_begin
                    , const World::model_instances_type::const_iterator& models_end
                    , const World::wmo_instances_type::const_iterator& wmos_begin
                    , const World::wmo_instances_type::const_iterator& wmos_end
                    );

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
