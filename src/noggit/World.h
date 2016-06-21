// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <map>
#include <string>
#include <cstdint>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <QImage>

#include <noggit/Model.h> // ModelManager
#include <noggit/Selection.h> // nameEntryManager
#include <noggit/Sky.h> // Skies, OutdoorLighting, OutdoorLightStats
#include <noggit/WMO.h> // WMOManager
#include <noggit/map_index.hpp>
#include <noggit/MapHeaders.h> // ENTRY_MODF
#include <math/ray.hpp>

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

struct world_selection_mask
{
  uint8_t terrain : 1;
  uint8_t model : 1;
  uint8_t map_object : 1;
};

struct low_res_batch
{
  low_res_batch ()
    : vertex_start (0)
    , vertex_count (0)
  {}

  low_res_batch (uint32_t _vertex_start, uint32_t _vertex_count)
    : vertex_start(_vertex_start)
    , vertex_count(_vertex_count)
  {}

  uint32_t vertex_start;
  uint32_t vertex_count;
};

//! \todo Split this. There should be a seperate class for WDLs.
class World
{
private:
  // --- stuff that should not be here. ----------------------------------------

  float time; //!< the time of the day

public:
  void advance_times ( const float& seconds
                     , const float& time_of_day_speed_factor
                     );

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

  std::unique_ptr<Skies> skies;

  //! \todo  Get these managed? ._.
  typedef std::pair<int, ModelInstance *> model_instance_type;
  typedef std::pair<int, WMOInstance *> wmo_instance_type;

  typedef std::map<int, ModelInstance *> model_instances_type;
  typedef std::map<int, WMOInstance *> wmo_instances_type;

  model_instances_type mModelInstances;
  wmo_instances_type mWMOInstances;

  OutdoorLightStats outdoorLightStats;

  void initDisplay();

  void tick(float dt);
  void draw ( size_t flags
            , float inner_cursor_radius
            , float outer_cursor_radius
            , const QPointF& mouse_position
            , const float& fog_distance
            , const boost::optional<selection_type>& selected_item
            );

  selection_result intersect (math::ray ray, world_selection_mask flags);

  void drawTileMode (bool draw_lines, float ratio, float zoom);

  void outdoorLights(bool on);
  void setupFog (bool draw_fog, const float& fog_distance);

  //! \brief Get the area ID of the tile on which the camera currently is on.
  unsigned int getAreaID() const;
  void setAreaID(int id, int x, int z);
  void setAreaID(int id, int x, int z , int cx, int cz);
  void setAreaID (int id, const ::math::vector_3d& position);
  void setFlag(bool to, float x, float z);
  void setBaseTexture(int x, int z, noggit::scoped_blp_texture_reference texture );

  boost::optional<float> get_height ( const float& x
                                    , const float& z
                                    ) const;

  void changeTerrain(float x, float z, float change, float radius, int BrushType);
  void flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  void blurTerrain(float x, float z, float remain, float radius, int BrushType);
  bool paintTexture(float x, float z, const brush& Brush, float strength, float pressure, noggit::scoped_blp_texture_reference texture);
  void eraseTextures(float x, float z);
  void overwriteTextureAtCurrentChunk( float x, float z, noggit::scoped_blp_texture_reference oldTexture, noggit::scoped_blp_texture_reference newTexture);
  void addHole (float x, float z, float h, bool whole_chunk);
  void removeHole (float x, float z, bool whole_chunk);

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

  static bool IsEditableWorld( int pMapId );
  void clearHeight(int x, int z);
  void moveHeight(int x, int z, const float& heightDelta);

  void clearAllModelsOnADT(int x, int z);

  noggit::map_index& map_index()
  {
    return _map_index;
  }

  const noggit::map_index& map_index() const
  {
    return _map_index;
  }

  const QImage& minimap() const { return _minimap; }

  bool canWaterSave (int x, int y) const;

  void setWaterHeight (int x, int y, float h);
  boost::optional<float> getWaterHeight (int x, int y) const;

  void setWaterTrans (int x, int y, unsigned char value);
  boost::optional<unsigned char> getWaterTrans(int x, int y) const;

  void setWaterType (int x, int y, int type);
  boost::optional<int> getWaterType (int x, int y) const;

  void deleteWaterLayer (int x,int z);

  void addWaterLayer (int x, int z);
  void addWaterLayer (int x, int z, float height, unsigned char trans);

  void autoGenWaterTrans (int x, int y, int factor);

private:
  QImage _minimap;

  bool _initialized_display;

  //! opengl buffer for the WDL low resolution heightmaps.
  GLuint _low_res_vertices_buffer;
  low_res_batch _low_res_batches[64][64];

  //! Vertex Buffer Objects for coordinates used for drawing.
  GLuint detailtexcoords;
  GLuint alphatexcoords;

  //! Map ID of this World.
  unsigned int mMapId;
  //! \brief Name of this map.
  std::string basename;

  //! The lighting used.
  std::unique_ptr<OutdoorLighting> ol;

  void initMinimap();
  void initLowresTerrain();

  void outdoorLighting();

  noggit::map_index _map_index;
};
