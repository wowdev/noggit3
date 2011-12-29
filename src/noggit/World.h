#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <string>

#include <QImage>

#include <noggit/Frustum.h> // Frustum
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


  // Which tile are we over / entering?
  int cx;
  int cz;
  int ex;
  int ez;

  // Holding all MapTiles there can be in a World.
  MapTileEntry mTiles[64][64];

  // Information about the currently selected model / WMO / triangle.
  nameEntry* mCurrentSelection;
  int mCurrentSelectedTriangle;
  bool SelectionMode;

  // Is the WDT telling us to use a different alphamap structure.
  bool mBigAlpha;

  // Call lists for the low resolution heightmaps.
  opengl::call_list* lowrestiles[64][64];

  // Temporary variables for loading a WMO, if we have a global WMO.
  std::string mWmoFilename;
  ENTRY_MODF mWmoEntry;

  // Vertex Buffer Objects for coordinates used for drawing.
  GLuint detailtexcoords;
  GLuint alphatexcoords;

  // Map ID of this World.
  unsigned int mMapId;

  // The lighting used.
  OutdoorLighting *ol;

  // Light attenuation related parameters.
  float l_const;
  float l_linear;
  float l_quadratic;


  void initMinimap();
  void initLowresTerrain();

  bool tileLoaded(int x, int z);
  MapTile *loadTile(int x, int z);

  void outdoorLighting();
  void outdoorLighting2();

public:
  unsigned int getMapID();

  // Time of the day.
  float animtime;
  float time;

  //! \brief Name of this map.
  std::string basename;

  // Dynamic distances for rendering. Actually, these should be the same..
  float fogdistance;
  float culldistance;

  bool autoheight;

  float minX;
  float maxX;
  float minY;
  float maxY;

  Skies *skies;
  bool mHasAGlobalWMO;
  bool noadt;

  //! \todo  Get these managed? ._.
  typedef std::pair<int, ModelInstance> model_instance_type;
  typedef std::pair<int, WMOInstance> wmo_instance_type;
  typedef std::map<int, ModelInstance> model_instances_type;
  typedef std::map<int, WMOInstance> wmo_instances_type;
  model_instances_type mModelInstances;
  wmo_instances_type mWMOInstances;

  OutdoorLightStats outdoorLightStats;

  Vec3D camera;
  Vec3D lookat;
  Frustum frustum;

  explicit World( const std::string& name);
  ~World();

  void initDisplay();
  void enterTile(int x, int z);
  void reloadTile(int x, int z);
  void saveTile(int x, int z);
  void saveChanged();
  void tick(float dt);
  //! \todo This seriously needs to be done via flags.
  void draw ( bool draw_terrain_height_contour
            , bool mark_impassable_chunks
            , bool draw_area_id_overlay
            , bool dont_draw_cursor
            , float inner_cursor_radius
            , float outer_cursor_radius
            , bool draw_wmo_doodads
            , bool draw_fog
            , bool draw_wmos
            , bool draw_terrain
            , bool draw_doodads
            , bool draw_lines
            , bool draw_hole_lines
            , bool draw_water
            );

  void outdoorLights(bool on);
  void setupFog (bool draw_fog);

  //! \brief Get the area ID of the tile on which the camera currently is on.
  unsigned int getAreaID();
  void setAreaID(int id, int x, int z);
  void setAreaID(int id, int x, int z , int cx, int cz);
  void setFlag(bool to, float x, float z);
  void setBaseTexture(int x, int z );

  void moveADT();

  void drawSelection ( int cursorX
                     , int cursorY
                     , bool draw_wmo_doodads
                     , bool draw_wmos
                     , bool draw_doodads
                     , bool draw_terrain
                     );
  void drawSelectionChunk(int cursorX,int cursorY);
  void drawTileMode ( bool draw_lines
                    , float ratio
                    , float zoom
                    );

  // Selection related methods.
private:
  void getSelection( );
public:
  bool HasSelection() { return mCurrentSelection; }
  bool IsSelection( int pSelectionType ) { return HasSelection() && mCurrentSelection->type == pSelectionType; }
  nameEntry * GetCurrentSelection() { return mCurrentSelection; }
  void ResetSelection() { mCurrentSelection = NULL; }
  GLuint GetCurrentSelectedTriangle() { return mCurrentSelectedTriangle; }

  bool GetVertex(float x,float z, Vec3D *V);
  void changeTerrain(float x, float z, float change, float radius, int BrushType);
  void flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  void blurTerrain(float x, float z, float remain, float radius, int BrushType);
  bool paintTexture(float x, float z, brush *Brush, float strength, float pressure, noggit::blp_texture* texture);
  void eraseTextures(float x, float z);
  void overwriteTextureAtCurrentChunk( float x, float z, noggit::blp_texture* oldTexture, noggit::blp_texture* newTexture);
  void addHole( float x, float z );

  void addModel ( nameEntry entry
                , Vec3D newPos
                , bool size_randomization
                , bool position_randomization
                , bool rotation_randomization
                );
  void addM2 ( Model *model
             , Vec3D newPos
             , bool size_randomization = false
             , bool position_randomization = false
             , bool rotation_randomization = false
             );
  void addWMO( WMO *wmo, Vec3D newPos );

  void removeHole( float x, float z );
  void jumpToCords(Vec3D pos);
  void saveMap();

  void setChanged(float x, float z);
  void setChanged(int x, int z);
  void unsetChanged(int x, int z);
  bool getChanged(int x, int z) const;

  void deleteModelInstance( int pUniqueID );
  void deleteWMOInstance( int pUniqueID );

  bool hasTile( int pX, int pZ ) const;

  static bool IsEditableWorld( int pMapId );
  void clearHeight(int id, int x, int z);
  void clearHeight(int id, int x, int z , int _cx, int _cz);
  void moveHeight(int id, int x, int z);
  void moveHeight(int id, int x, int z , int _cx, int _cz);

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
