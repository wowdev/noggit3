// World.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <string>
#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <QImage>

#include <noggit/Model.h> // ModelManager
#include <noggit/Selection.h> // nameEntryManager
#include <noggit/Sky.h> // Skies, OutdoorLighting, OutdoorLightStats
#include <noggit/WMO.h> // WMOManager
#include <noggit/MapHeaders.h> // ENTRY_MODF

namespace OpenGL
{
  class Texture;
};
namespace opengl
{
  class call_list;
};

class brush;
class MapTile;

static const float detail_size = 8.0f;
static const float highresdistance = 384.0f;
static const float mapdrawdistance = 998.0f;
static const float modeldrawdistance = 384.0f;
static const float doodaddrawdistance = 64.0f;

typedef unsigned short StripType;

/*!
 \brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing. This is private and for the class World only.
 */
class MapTileEntry
{
private:
  uint32_t flags;
  MapTile* tile;

  MapTileEntry() : flags( 0 ), tile( NULL ) {}

  friend class World;
};

//! \todo Split this. There should be a seperate class for WDTs.
class World
{
private:
  // --- stuff that should not be here. ----------------------------------------

  float time; //!< the time of the day

  void unsetChanged(int x, int z);

  bool _tile_got_modified[64][64];

public:
  void setChanged(float x, float z);
  void setChanged(int x, int z);
  void advance_times ( const float& seconds
                     , const float& time_of_day_speed_factor
                     );
  bool getChanged(int x, int z) const;

  ::math::vector_3d _exact_terrain_selection_position;
  ::math::vector_3d camera;
  ::math::vector_3d lookat;

  // --- stuff that should be in here. -----------------------------------------

public:
  explicit World( const std::string& name);
  ~World();

  //! gets the current MapID
  /*!
  \return the MapID found in dbcs
  */
  const unsigned int& getMapID() const;

  Skies *skies;

  //! \todo  Get these managed? ._.
  typedef std::pair<int, ModelInstance *> model_instance_type;
  typedef std::pair<int, WMOInstance *> wmo_instance_type;
  typedef std::map<int, ModelInstance *> model_instances_type;
  typedef std::map<int, WMOInstance *> wmo_instances_type;
  model_instances_type mModelInstances;
  wmo_instances_type mWMOInstances;

  OutdoorLightStats outdoorLightStats;

  void initDisplay();
  void load_tiles_around ( const size_t& x
                         , const size_t& z
                         , const size_t& distance
                         );
  void reloadTile(int x, int z);
  void saveTile(int x, int z) const;
  void saveTileCata(int x, int z) const;
  void saveChanged();
  void tick(float dt);
  void draw ( size_t flags
            , float inner_cursor_radius
            , float outer_cursor_radius
            , const QPointF& mouse_position
            , const float& fog_distance
            , const boost::optional<selection_type>& selected_item
            );

  boost::optional<selection_type> drawSelection (size_t flags);
  void drawTileMode (bool draw_lines, float ratio, float zoom);

  void outdoorLights(bool on);
  void setupFog (bool draw_fog, const float& fog_distance);

  //! \brief Get the area ID of the tile on which the camera currently is on.
  unsigned int getAreaID() const;
  void setAreaID(int id, int x, int z);
  void setAreaID(int id, int x, int z , int cx, int cz);
  void setAreaID (int id, const ::math::vector_3d& position);
  void setFlag(bool to, float x, float z);
  void setBaseTexture(int x, int z, noggit::blp_texture* texture );

  boost::optional<float> get_height ( const float& x
                                    , const float& z
                                    ) const;

  void changeTerrain(float x, float z, float change, float radius, int BrushType);
  void flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  void blurTerrain(float x, float z, float remain, float radius, int BrushType);
  bool paintTexture(float x, float z, const brush& Brush, float strength, float pressure, noggit::blp_texture* texture);
  void eraseTextures(float x, float z);
  void overwriteTextureAtCurrentChunk( float x, float z, noggit::blp_texture* oldTexture, noggit::blp_texture* newTexture);
  void addHole (float x, float z, bool whole_chunk);
  void removeHole (float x, float z, bool whole_chunk);

  void addModel ( const nameEntry& entry
                , ::math::vector_3d newPos
                , bool size_randomization
                , bool position_randomization
                , bool rotation_randomization
                );
  void addM2 ( std::string const& path
             , ::math::vector_3d newPos
             , bool size_randomization = false
             , bool position_randomization = false
             , bool rotation_randomization = false
             );
  void addWMO (std::string const& path, ::math::vector_3d newPos );

  void saveMap();

  void deleteModelInstance( int pUniqueID );
  void deleteWMOInstance( int pUniqueID );

  bool hasTile( int pX, int pZ ) const;

  static bool IsEditableWorld( int pMapId );
  void clearHeight(int x, int z);
  void moveHeight(int x, int z, const float& heightDelta);

  void saveWDT();
  void clearAllModelsOnADT(int x, int z);

  nameEntryManager& selection_names()
  {
    return _selection_names;
  }

  const QImage& minimap() const { return _minimap; }

private:
  QImage _minimap;

  //! Holding all MapTiles there can be in a World.
  MapTileEntry mTiles[64][64];

  //! Is the WDT telling us to use a bigger alphamap (64*64) and single pass rendering.
  bool mBigAlpha;

  //! opengl call lists for the WDL low resolution heightmaps.
  opengl::call_list* lowrestiles[64][64];

  //! Vertex Buffer Objects for coordinates used for drawing.
  GLuint detailtexcoords;
  GLuint alphatexcoords;

  //! Map ID of this World.
  unsigned int mMapId;
  //! \brief Name of this map.
  std::string basename;

  //! The lighting used.
  OutdoorLighting *ol;

  void initMinimap();
  void initLowresTerrain();

  //! Checks if a maptile is loaded
  bool tileLoaded(int x, int z) const;

  //! loads a maptile if isnt already
  MapTile *loadTile(int x, int z);

  void outdoorLighting();

  nameEntryManager _selection_names;
};

#endif
