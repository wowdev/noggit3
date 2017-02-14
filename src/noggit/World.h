// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <noggit/Frustum.h> // Frustum
#include <noggit/Model.h> // ModelManager
#include <noggit/Selection.h>
#include <noggit/Sky.h> // Skies, OutdoorLighting, OutdoorLightStats
#include <noggit/WMO.h> // WMOManager
#include <noggit/tile_index.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/map_horizon.h>

#include <map>
#include <string>
#include <unordered_set>

namespace opengl
{
  class call_list;
}

class Brush;
class MapTile;
class MapIndex;

static const float detail_size = 8.0f;
static const float highresdistance = 384.0f;
static const float mapdrawdistance = 998.0f;
static const float modeldrawdistance = 384.0f;
static const float doodaddrawdistance = 64.0f;

using StripType = uint16_t;

class World
{
public:
  MapIndex *mapIndex;
  map_horizon horizon;

  // Information about the currently selected model / WMO / triangle.
  boost::optional<selection_type> mCurrentSelection;
  bool SelectionMode;

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

  void initMinimap();

  void outdoorLighting();
  void outdoorLighting2();

  unsigned int getMapID();
  // Do we draw *? Should be moved somewhere else, these are not World related.
  bool drawfog;
  // Time of the day.
  float animtime;
  float time;

  //! \brief Name of this map.
  std::string basename;

  // Dynamic distances for rendering. Actually, these should be the same..
  float fogdistance;
  float culldistance;

  Skies *skies;

  //! \todo  Get these managed? ._.
  std::map<int, ModelInstance> mModelInstances;
  std::map<int, WMOInstance> mWMOInstances;

  OutdoorLightStats outdoorLightStats;

  StripType *mapstrip2;

  math::vector_3d camera;

  explicit World(const std::string& name);
  ~World();

  void initDisplay();

  void tick(float dt);
  void draw ( math::vector_3d const& cursor_pos
            , math::vector_4d const& cursor_color
            , int cursor_type
            , float brushRadius
            , float hardness
            , bool highlightPaintableChunks
            , bool draw_contour
            , float innerRadius
            , math::vector_3d const& ref_pos
            , float angle
            , float orientation
            , bool use_ref_pos
            , bool angled_mode
            , bool draw_paintability_overlay
            , bool draw_chunk_flag_overlay
            , bool draw_water_overlay
            , bool draw_areaid_overlay
            //! \todo passing editing_mode is _so_ wrong, I don't believe I'm doing this
            , editing_mode
            , math::vector_3d const& camera_pos
            , math::vector_3d const& camera_lookat
            , bool draw_mfbo
            , bool draw_wireframe
            , bool draw_lines
            , bool draw_terrain
            , bool draw_wmo
            , bool draw_water
            , bool draw_doodads
            , bool draw_models
            , bool draw_model_animations
            );

  void outdoorLights(bool on);
  void setupFog();

  unsigned int getAreaID (math::vector_3d const&);
  void setAreaID(math::vector_3d const& pos, int id, bool adt);

  selection_result intersect ( math::ray const&
                             , bool only_map
                             , bool do_objects
                             , bool draw_terrain
                             , bool draw_wmo
                             , bool draw_models
                             );
  void drawTileMode ( float ah
                    , math::vector_3d const& camera_pos
                    , bool draw_lines
                    , float zoom
                    );

  void initGlobalVBOs(GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords);

  bool HasSelection();

  // Selection related methods.
  bool IsSelection(int pSelectionType);
  boost::optional<selection_type> GetCurrentSelection() { return mCurrentSelection; }
  void SetCurrentSelection (boost::optional<selection_type> entry) { mCurrentSelection = entry; }
  void ResetSelection() { mCurrentSelection.reset(); }

  bool GetVertex(float x, float z, math::vector_3d *V);

  // check if the cursor is under map or in an unloaded tile
  bool isUnderMap(math::vector_3d const& pos);

  template<typename Fun>
    bool for_all_chunks_in_range ( math::vector_3d const& pos
                                 , float radius
                                 , Fun&& /* MapChunk* -> bool changed */
                                 );
  template<typename Fun, typename Post>
    bool for_all_chunks_in_range ( math::vector_3d const& pos
                                 , float radius
                                 , Fun&& /* MapChunk* -> bool changed */
                                 , Post&& /* MapChunk* -> void; called for all changed chunks */
                                 );
  template<typename Fun>
    void for_all_chunks_on_tile (math::vector_3d const& pos, Fun&&);

  template<typename Fun>
    void for_chunk_at(math::vector_3d const& pos, Fun&&);

  template<typename Fun>
    void for_tile_at(math::vector_3d const& pos, Fun&&);

  void changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius);
  void changeShader(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode);
  void flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, int flattenType, const math::vector_3d& origin, math::degrees angle, math::degrees orientation);
  void blurTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType);
  bool paintTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool sprayTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, scoped_blp_texture_reference texture);

  void eraseTextures(math::vector_3d const& pos);
  void overwriteTextureAtCurrentChunk(math::vector_3d const& pos, scoped_blp_texture_reference oldTexture, scoped_blp_texture_reference newTexture);
  void setBaseTexture(math::vector_3d const& pos);
  void swapTexture(math::vector_3d const& pos, scoped_blp_texture_reference tex);
  void removeTexDuplicateOnADT(math::vector_3d const& pos);

  void setHole(math::vector_3d const& pos, bool big, bool hole);
  void setHoleADT(math::vector_3d const& pos, bool hole);

  void addModel(selection_type, math::vector_3d newPos, bool copyit);
  void addM2(std::string const& filename, math::vector_3d newPos, bool copyit);
  void addWMO(std::string const& filename, math::vector_3d newPos, bool copyit);


  void updateTilesEntry(selection_type const& entry);
  void updateTilesWMO(WMOInstance* wmo);
  void updateTilesModel(ModelInstance* m2);

  std::unordered_set<WMO*> _hidden_map_objects;
  std::unordered_set<Model*> _hidden_models;
  void clearHiddenModelList();

  void saveMap();

  void deleteModelInstance(int pUniqueID);
  void deleteWMOInstance(int pUniqueID);

  void delete_duplicate_model_and_wmo_instances();

	static bool IsEditableWorld(int pMapId);

  void clearHeight(math::vector_3d const& pos);
  void clearAllModelsOnADT(math::vector_3d const& pos);

  // liquids
  void paintLiquid( math::vector_3d const& pos
                  , float radius
                  , int liquid_id
                  , bool add
                  , math::radians const& angle
                  , math::radians const& orientation
                  , bool lock
                  , math::vector_3d const& origin
                  , bool override_height
                  , bool override_liquid_id
                  , float opacity_factor
                  );
  bool canWaterSave(const tile_index& tile);
  void CropWaterADT(math::vector_3d const& pos);
  void setWaterType(math::vector_3d const& pos, int type);
  int getWaterType(const tile_index& tile);
  void autoGenWaterTrans(math::vector_3d const&, float factor);


  void fixAllGaps();

  void convert_alphamap(bool to_big_alpha);

  void deselectVertices(math::vector_3d const& pos, float radius);
  void selectVertices(math::vector_3d const& pos, float radius);

  void moveVertices(float h);
  void orientVertices(math::vector_3d const& ref_pos);
  void flattenVertices();

  void updateSelectedVertices();
  void updateVertexCenter();
  void clearVertexSelection();

  math::vector_3d& vertexCenter();

  math::degrees vertex_angle;
  math::degrees vertex_orientation;

private:
  void getSelection();

  std::set<MapChunk*>& vertexBorderChunks();

  std::set<MapTile*> _vertex_tiles;
  std::set<MapChunk*> _vertex_chunks;
  std::set<MapChunk*> _vertex_border_chunks;
  std::set<math::vector_3d*> _vertices_selected;
  math::vector_3d _vertex_center;
  bool _vertex_center_updated = false;
  bool _vertex_border_updated = false;
};

extern World *gWorld;
