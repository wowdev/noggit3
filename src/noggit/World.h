// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/frustum.hpp>
#include <math/trig.hpp>
#include <noggit/cursor_render.hpp>
#include <noggit/Misc.h>
#include <noggit/Model.h> // ModelManager
#include <noggit/Selection.h>
#include <noggit/Sky.h> // Skies, OutdoorLighting, OutdoorLightStats
#include <noggit/WMO.h> // WMOManager
#include <noggit/map_horizon.h>
#include <noggit/map_index.hpp>
#include <noggit/tile_index.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/world_tile_update_queue.hpp>
#include <noggit/world_model_instances_storage.hpp>
#include <opengl/primitives.hpp>
#include <opengl/shader.fwd.hpp>

#include <boost/optional/optional.hpp>

#include <QtCore/QSettings>

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

namespace noggit
{
  struct object_paste_params;
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
private:
  std::unordered_map<std::string, std::vector<ModelInstance*>> _models_by_filename;
  noggit::world_model_instances_storage _model_instance_storage;
  noggit::world_tile_update_queue _tile_update_queue;
public:
  MapIndex mapIndex;
  noggit::map_horizon horizon;

  // Temporary variables for loading a WMO, if we have a global WMO.
  std::string mWmoFilename;
  ENTRY_MODF mWmoEntry;

  // Vertex Buffer Objects for coordinates used for drawing.
  GLuint detailtexcoords;
  GLuint alphatexcoords;

  // The lighting used.
  std::unique_ptr<OutdoorLighting> ol;

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

  OutdoorLightStats outdoorLightStats;

  explicit World(const std::string& name, int map_id);

  void initDisplay();

  void update_models_emitters(float dt);
  void draw ( math::matrix_4x4 const& model_view
            , math::matrix_4x4 const& projection
            , math::vector_3d const& cursor_pos
            , math::vector_4d const& cursor_color
            , int cursor_type
            , float brush_radius
            , bool show_unpaintable_chunks
            , bool draw_contour
            , float inner_radius_ratio
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
            , bool camera_moved
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
            , bool draw_hidden_models
            , std::map<int, misc::random_color>& area_id_colors
            , bool draw_fog
            , eTerrainType ground_editing_brush
            , int water_layer
            , display_mode display
            );

  unsigned int getAreaID (math::vector_3d const&);
  void setAreaID(math::vector_3d const& pos, int id, bool adt);

  selection_result intersect ( math::matrix_4x4 const& model_view
                             , math::ray const&
                             , bool only_map
                             , bool do_objects
                             , bool draw_terrain
                             , bool draw_wmo
                             , bool draw_models
                             , bool draw_hidden_models
                             );

  void initGlobalVBOs(GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords);

private:
  // Information about the currently selected model / WMO / triangle.
  std::vector<selection_type> _current_selection;
  boost::optional<math::vector_3d> _multi_select_pivot;
  int _selected_model_count = 0;
  void update_selection_pivot();
public:

  boost::optional<math::vector_3d> const& multi_select_pivot() const { return _multi_select_pivot; }

  // Selection related methods.
  bool is_selected(selection_type selection) const;
  bool is_selected(std::uint32_t uid) const;
  std::vector<selection_type> const& current_selection() const { return _current_selection; }
  boost::optional<selection_type> get_last_selected_model() const;
  bool has_selection() const { return !_current_selection.empty(); }
  bool has_multiple_model_selected() const { return _selected_model_count > 1; }
  void set_current_selection(selection_type entry);
  void add_to_selection(selection_type entry);
  void remove_from_selection(selection_type entry);
  void remove_from_selection(std::uint32_t uid);
  void reset_selection();
  void delete_selected_models();

  enum class m2_scaling_type
  {
    set,
    add,
    mult
  };

  void snap_selected_models_to_the_ground();
  void scale_selected_models(float v, m2_scaling_type type);
  void move_selected_models(float dx, float dy, float dz);
  void move_selected_models(math::vector_3d const& delta)
  {
    move_selected_models(delta.x, delta.y, delta.z);
  }
  void set_selected_models_pos(float x, float y, float z, bool change_height = true)
  {
    return set_selected_models_pos({x,y,z}, change_height);
  }
  void set_selected_models_pos(math::vector_3d const& pos, bool change_height = true);
  void rotate_selected_models(math::degrees rx, math::degrees ry, math::degrees rz, bool use_pivot);
  void rotate_selected_models_randomly(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
  void set_selected_models_rotation(math::degrees rx, math::degrees ry, math::degrees rz);
  // Checks the normal of the terrain on model origin and rotates to that spot.
  void rotate_selected_models_to_ground_normal(bool smoothNormals);

  bool GetVertex(float x, float z, math::vector_3d *V) const;
  boost::optional<float> get_exact_height_at(math::vector_3d const& pos);

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
    void for_chunk_at(math::vector_3d const& pos, Fun&& fun);
  template<typename Fun>
    auto for_maybe_chunk_at (math::vector_3d const& pos, Fun&& fun) -> boost::optional<decltype (fun (nullptr))>;

  template<typename Fun>
    void for_tile_at(const tile_index& pos, Fun&&);

  MapChunk * get_chunk_at(math::vector_3d const& pos);

  void changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius);
  void changeShader(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode);
  math::vector_3d pickShaderColor(math::vector_3d const& pos);
  void flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode, const math::vector_3d& origin, math::degrees angle, math::degrees orientation);
  void blurTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode);
  bool paintTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool sprayTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, scoped_blp_texture_reference texture);
  bool replaceTexture(math::vector_3d const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture);

  void eraseTextures(math::vector_3d const& pos);
  void overwriteTextureAtCurrentChunk(math::vector_3d const& pos, scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture);
  void setBaseTexture(math::vector_3d const& pos);
  void clear_shadows(math::vector_3d const& pos);
  void clearTextures(math::vector_3d const& pos);
  void swapTexture(math::vector_3d const& pos, scoped_blp_texture_reference tex);
  void removeTexDuplicateOnADT(math::vector_3d const& pos);
  void change_texture_flag(math::vector_3d const& pos, scoped_blp_texture_reference const& tex, std::size_t flag, bool add);

  void setHole(math::vector_3d const& pos, bool big, bool hole);
  void setHoleADT(math::vector_3d const& pos, bool hole);

  ModelInstance* addM2 ( std::string const& filename
             , math::vector_3d newPos
             , float scale, math::degrees::vec3 rotation
             , noggit::object_paste_params*
             );
  WMOInstance* addWMO ( std::string const& filename
              , math::vector_3d newPos
              , math::degrees::vec3 rotation
              );

  // add a m2 instance to the world (needs to be positioned already), return the uid
  std::uint32_t add_model_instance(ModelInstance model_instance, bool from_reloading);
  // add a wmo instance to the world (needs to be positioned already), return the uid
  std::uint32_t add_wmo_instance(WMOInstance wmo_instance, bool from_reloading);

  boost::optional<selection_type> get_model(std::uint32_t uid);
  void remove_models_if_needed(std::vector<uint32_t> const& uids);

  void reload_tile(tile_index const& tile);

  void updateTilesEntry(selection_type const& entry, model_update type);
  void updateTilesWMO(WMOInstance* wmo, model_update type);
  void updateTilesModel(ModelInstance* m2, model_update type);
  void wait_for_all_tile_updates();

  void saveMap (int width, int height);

  void deleteModelInstance(int pUniqueID);
  void deleteWMOInstance(int pUniqueID);

  bool uid_duplicates_found() const;
  void delete_duplicate_model_and_wmo_instances();
  // used after the uid fix all
  void unload_every_model_and_wmo_instance();

	static bool IsEditableWorld(int pMapId);

  void clearHeight(math::vector_3d const& pos);
  void clearAllModelsOnADT(tile_index const& tile);

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
  void CropWaterADT(const tile_index& pos);
  void setWaterType(const tile_index& pos, int type, int layer);
  int getWaterType(const tile_index& tile, int layer);
  void autoGenWaterTrans(const tile_index&, float factor);


  void fixAllGaps();

  void convert_alphamap(bool to_big_alpha);

  bool deselectVertices(math::vector_3d const& pos, float radius);
  void selectVertices(math::vector_3d const& pos, float radius);
  void delete_models(std::vector<selection_type> const& types);
  void selectVertices(math::vector_3d const& pos1, math::vector_3d const& pos2);
  std::set<math::vector_3d*>* getSelectedVertices();

  template<typename Fun>
  bool for_all_chunks_between ( math::vector_3d const& pos1,
                                math::vector_3d const& pos2,
                                Fun&& /* MapChunk* -> bool changed */
                                );

  void select_all_chunks_between(math::vector_3d const& pos1, math::vector_3d const& pos2, std::vector<MapChunk*>& chunks_in);

  template<typename Fun>
  void for_each_wmo_instance(Fun&& function)
  {
    _model_instance_storage.for_each_wmo_instance(function);
  }

  template<typename Fun>
  void for_each_m2_instance(Fun&& function)
  {
    _model_instance_storage.for_each_m2_instance(function);
  }

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

  bool need_model_updates = false;

private:
  void update_models_by_filename();

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

  QSettings* _settings;

  float _view_distance;

  std::unique_ptr<opengl::program> _mcnk_program;;
  std::unique_ptr<opengl::program> _mfbo_program;
  std::unique_ptr<opengl::program> _m2_program;
  std::unique_ptr<opengl::program> _m2_instanced_program;
  std::unique_ptr<opengl::program> _m2_particles_program;
  std::unique_ptr<opengl::program> _m2_ribbons_program;
  std::unique_ptr<opengl::program> _m2_box_program;
  std::unique_ptr<opengl::program> _wmo_program;

  noggit::cursor_render _cursor_render;
  opengl::primitives::sphere _sphere_render;
  opengl::primitives::square _square_render;

  boost::optional<liquid_render> _liquid_render = boost::none;
};
