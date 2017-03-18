// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/frustum.hpp>
#include <math/trig.hpp>
#include <noggit/Misc.h>
#include <noggit/Model.h> // ModelManager
#include <noggit/Selection.h>
#include <noggit/Sky.h> // Skies, OutdoorLighting, OutdoorLightStats
#include <noggit/WMO.h> // WMOManager
#include <noggit/map_horizon.h>
#include <noggit/map_index.hpp>
#include <noggit/tile_index.hpp>
#include <noggit/tool_enums.hpp>

#include <map>
#include <string>
#include <unordered_set>

namespace opengl
{
  class call_list;
}

class Brush;
class MapTile;

static const float detail_size = 8.0f;
static const float highresdistance = 384.0f;
static const float modeldrawdistance = 384.0f;
static const float doodaddrawdistance = 64.0f;

using StripType = uint16_t;

class World
{
public:
  MapIndex mapIndex;
  noggit::map_horizon horizon;

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
  std::unique_ptr<OutdoorLighting> ol;

  void outdoorLighting();

  unsigned int getMapID();
  // Time of the day.
  float animtime;
  float time;

  //! \brief Name of this map.
  std::string basename;

  // Dynamic distances for rendering. Actually, these should be the same..
  float fogdistance;
  float culldistance;

  std::unique_ptr<Skies> skies;

  //! \todo  Get these managed? ._.
  std::map<int, ModelInstance> mModelInstances;
  std::map<int, WMOInstance> mWMOInstances;

  OutdoorLightStats outdoorLightStats;

  explicit World(const std::string& name);

  void initDisplay();

  void tick(float dt);
  void draw ( math::vector_3d const& cursor_pos
            , math::vector_4d const& cursor_color
            , int cursor_type
            , float brushRadius
            , float hardness
            , bool show_unpaintable_chunks
            , bool draw_contour
            , float innerRadius
            , math::vector_3d const& ref_pos
            , float angle
            , float orientation
            , bool use_ref_pos
            , bool angled_mode
            , bool draw_paintability_overlay
            , bool draw_chunk_flag_overlay
            , bool draw_areaid_overlay
            //! \todo passing editing_mode is _so_ wrong, I don't believe I'm doing this
            , editing_mode
            , math::vector_3d const& camera_pos
            , bool draw_mfbo
            , bool draw_wireframe
            , bool draw_lines
            , bool draw_terrain
            , bool draw_wmo
            , bool draw_water
            , bool draw_doodads
            , bool draw_models
            , bool draw_model_animations
            , bool draw_hole_lines
            , bool draw_models_with_box
            , std::unordered_set<WMO*> const& hidden_map_objects
            , std::unordered_set<Model*> const& hidden_models
            , std::map<int, misc::random_color>& area_id_colors
            , bool draw_fog
            );

  void outdoorLights(bool on);
  void setupFog (bool draw_fog);

  unsigned int getAreaID (math::vector_3d const&);
  void setAreaID(math::vector_3d const& pos, int id, bool adt);

  selection_result intersect ( math::ray const&
                             , bool only_map
                             , bool do_objects
                             , bool draw_terrain
                             , bool draw_wmo
                             , bool draw_models
                             , std::unordered_set<WMO*> const& hidden_map_objects
                             , std::unordered_set<Model*> const& hidden_models
                             );
  void drawTileMode ( float ah
                    , math::vector_3d const& camera_pos
                    , bool draw_lines
                    , float zoom
                    , float aspect_ratio
                    );

  void initGlobalVBOs(GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords);

  bool HasSelection();

  // Selection related methods.
  bool IsSelection(int pSelectionType);
  boost::optional<selection_type> GetCurrentSelection() { return mCurrentSelection; }
  void SetCurrentSelection (boost::optional<selection_type> entry) { mCurrentSelection = entry; }
  void ResetSelection() { mCurrentSelection.reset(); }

  bool GetVertex(float x, float z, math::vector_3d *V) const;

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
    auto for_chunk_at(math::vector_3d const& pos, Fun&& fun) -> decltype (fun (nullptr));
  template<typename Fun>
    auto for_maybe_chunk_at (math::vector_3d const& pos, Fun&& fun) -> boost::optional<decltype (fun (nullptr))>;

  template<typename Fun>
    void for_tile_at(const tile_index& pos, Fun&&);

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

  void saveMap (int width, int height);

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
  void CropWaterADT(const tile_index& pos);
  void setWaterType(const tile_index& pos, int type);
  int getWaterType(const tile_index& tile);
  void autoGenWaterTrans(const tile_index&, float factor);


  void fixAllGaps();

  void convert_alphamap(bool to_big_alpha);

  bool deselectVertices(math::vector_3d const& pos, float radius);
  void selectVertices(math::vector_3d const& pos, float radius);

  void moveVertices(float h);
  void orientVertices ( math::vector_3d const& ref_pos
                      , math::degrees vertex_angle
                      , math::degrees vertex_orientation
                      );
  void flattenVertices (float height);

  void updateSelectedVertices();
  void updateVertexCenter();
  void clearVertexSelection();

  math::vector_3d const& vertexCenter();

  void recalc_norms (MapChunk*) const;

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

  std::unique_ptr<noggit::map_horizon::render> _horizon_render;

  bool _display_initialized = false;
};

extern World *gWorld;
