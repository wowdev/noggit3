// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <vector>
#include <map>
#include <string>

#include <boost/optional.hpp>

#include <opengl/shader.fwd.hpp>
#include <opengl/types.h>

#include <noggit/MapHeaders.h>
#include <noggit/TileWater.hpp>
#include <noggit/World.h> // instances types

class Frustum;
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
  void drawMFBO (opengl::scoped::use_program&);

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
  math::vector_3d mMinimumValues[3 * 3];
  math::vector_3d mMaximumValues[3 * 3];

  // MHDR:
  int mFlags;
  bool mBigAlpha;

  // Data to be loaded and later unloaded.
  std::vector<std::string> mTextureFilenames;
  std::vector<std::string> mModelFilenames;
  std::vector<std::string> mWMOFilenames;

  std::string mFilename;

  MapChunk * mChunks[16][16];

  World* _world;

  noggit::TileWater _water;

  friend class texture_set;
  friend class MapChunk;
  friend class World;
};
