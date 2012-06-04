// World.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
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
public:
  const QImage& minimap() const { return _minimap; }

private:
  QImage _minimap;

  int cx; //!< camera x-coord
  int cz; //!< camera z-coord

  int ex; //!< maptile x-coord
  int ez; //!< maptile z-coord

  //! Holding all MapTiles there can be in a World.
  MapTileEntry mTiles[64][64];

  //! Information about the currently selected model / WMO / triangle.
  nameEntry* mCurrentSelection;
  int mCurrentSelectedTriangle;
  bool SelectionMode;

  //! Is the WDT telling us to use a bigger alphamap (64*64) and single pass rendering.
  bool mBigAlpha;

  //! opengl call lists for the WDL low resolution heightmaps.
  opengl::call_list* lowrestiles[64][64];

  //! Temporary variables for loading a WMO, if we have a global WMO.
  std::string mWmoFilename;
  ENTRY_MODF mWmoEntry;

  //! Vertex Buffer Objects for coordinates used for drawing.
  GLuint detailtexcoords;
  GLuint alphatexcoords;

  //! Map ID of this World.
  unsigned int mMapId;

  //! The lighting used.
  OutdoorLighting *ol;

  //! Light attenuation related parameters.
  float l_const;
  float l_linear;
  float l_quadratic;

  float time; //!< the time of the day

  void initMinimap();
  void initLowresTerrain();

  //! \brief Name of this map.
  std::string basename;

  //! Checks if a maptile is loaded
  /*!
  \param x a integer indecating the x coord
  \param z a integer indecating the z coord
  \return true when tile is loaded
  */
  bool tileLoaded(int x, int z) const;

  //! loads a maptile if isnt already
  /*!
  \param x a integer indecating the x coord
  \param z a integer indecating the z coord
  \return the corresponding MapTile object
  */
  MapTile *loadTile(int x, int z);

  void outdoorLighting();
  void outdoorLighting2();

public:

  void advance_times ( const float& seconds
                     , const float& time_of_day_speed_factor
                     );

  //! gets the current MapID
  /*!
  \return the MapID found in dbcs
  */
  const unsigned int getMapID() const;

  void set_camera_above_terrain();

  Skies *skies;
  bool mHasAGlobalWMO;
  bool noadt;

  //! \todo  Get these managed? ._.
  typedef std::pair<int, ModelInstance *> model_instance_type;
  typedef std::pair<int, WMOInstance *> wmo_instance_type;
  typedef std::map<int, ModelInstance *> model_instances_type;
  typedef std::map<int, WMOInstance *> wmo_instances_type;
  model_instances_type mModelInstances;
  wmo_instances_type mWMOInstances;

  OutdoorLightStats outdoorLightStats;

  ::math::vector_3d camera;
  ::math::vector_3d lookat;

  explicit World( const std::string& name);
  ~World();

  void initDisplay();
  void enterTile(int x, int z);
  void reloadTile(int x, int z);
  void saveTile(int x, int z);
  void saveTileCata(int x, int z);
  void saveChanged();
  void tick(float dt);
  void draw ( size_t flags
            , float inner_cursor_radius
            , float outer_cursor_radius
            , const QPointF& mouse_position
            , const float& fog_distance
            );

  void drawSelection (size_t flags);
  void drawSelectionChunk(int cursorX,int cursorY);
  void drawTileMode ( bool draw_lines
                    , float ratio
                    , float zoom
                    );

  void outdoorLights(bool on);
  void setupFog (bool draw_fog, const float& fog_distance);

  //! \brief Get the area ID of the tile on which the camera currently is on.
  const unsigned int getAreaID() const;
  void setAreaID(int id, int x, int z);
  void setAreaID(int id, int x, int z , int cx, int cz);
  void setFlag(bool to, float x, float z);
  void setBaseTexture(int x, int z, noggit::blp_texture* texture );

  void moveADT();

  bool HasSelection() { return mCurrentSelection; }
  bool IsSelection( int pSelectionType ) { return HasSelection() && mCurrentSelection->type == pSelectionType; }
  nameEntry * GetCurrentSelection() { return mCurrentSelection; }
  void ResetSelection() { mCurrentSelection = NULL; }
  GLuint GetCurrentSelectedTriangle() { return mCurrentSelectedTriangle; }

  ::math::vector_3d _exact_terrain_selection_position;

  bool GetVertex(float x,float z, ::math::vector_3d *V);
  boost::optional<float> get_height (const float& x, const float& z) const;

  void changeTerrain(float x, float z, float change, float radius, int BrushType);
  void flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  void blurTerrain(float x, float z, float remain, float radius, int BrushType);
  bool paintTexture(float x, float z, const brush& Brush, float strength, float pressure, noggit::blp_texture* texture);
  void eraseTextures(float x, float z);
  void overwriteTextureAtCurrentChunk( float x, float z, noggit::blp_texture* oldTexture, noggit::blp_texture* newTexture);
  void addHole( float x, float z );

  void addModel ( const nameEntry& entry
                , ::math::vector_3d newPos
                , bool size_randomization
                , bool position_randomization
                , bool rotation_randomization
                );
  void addM2 ( Model *model
             , ::math::vector_3d newPos
             , bool size_randomization = false
             , bool position_randomization = false
             , bool rotation_randomization = false
             );
  void addWMO( WMO *wmo, ::math::vector_3d newPos );

  void removeHole( float x, float z );
  void jumpToCords(::math::vector_3d pos);
  void saveMap();

  void setChanged(float x, float z);
  void setChanged(int x, int z);
  void unsetChanged(int x, int z);
  bool getChanged(int x, int z) const;

  void deleteModelInstance( int pUniqueID );
  void deleteWMOInstance( int pUniqueID );

  bool hasTile( int pX, int pZ ) const;

  static bool IsEditableWorld( int pMapId );
  void clearHeight(int x, int z);
  void moveHeight(int x, int z);

  void saveWDT();
  void clearAllModelsOnADT(int x, int z);

  nameEntryManager& selection_names()
  {
    return _selection_names;
  }

private:
  nameEntryManager _selection_names;

  GLuint _selection_buffer[8192];
};

#endif
