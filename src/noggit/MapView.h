// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_4d.hpp>
#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/bool_toggle_property.hpp>
#include <noggit/camera.hpp>
#include <noggit/tool_enums.hpp>

#include <boost/optional.hpp>

#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QOpenGLWidget>

#include <forward_list>
#include <map>
#include <unordered_set>

#ifdef _WIN32
#include <external/wacom/WINTAB.h>
#endif


class World;


enum eViewMode
{
  eViewMode_Minimap,
  eViewMode_2D,
  eViewMode_3D
};

class WMO;
class Model;

namespace noggit
{
  class camera;
  namespace ui
  {
    class current_texture;
    class cursor_switcher;
    class detail_infos;
    class exit_warning;
    class flatten_blur_tool;
    class help;
    class helper_models;
    class minimap_widget;
    class object_editor;
    class shader_tool;
    class terrain_tool;
    class texture_picker;
    class texturing_tool;
    class toolbar;    
    class water;
    class water_save_warning;
    class zone_id_browser;
    struct main_window;
    struct tileset_chooser;
  }
}


class MapView : public QOpenGLWidget
{
private:
  bool _mod_alt_down = false;
  bool _mod_ctrl_down = false;
  bool _mod_shift_down = false;
  bool _mod_space_down = false;

  float _2d_zoom = 0.25f;
  float moving, strafing, updown, mousedir, turn, lookat;
  math::vector_3d _cursor_pos;
  bool key_w;
  bool look;

  noggit::camera _camera;

  noggit::bool_toggle_property _draw_contour = {false};
  noggit::bool_toggle_property _draw_mfbo = {false};
  noggit::bool_toggle_property _draw_wireframe = {false};
  noggit::bool_toggle_property _draw_lines = {false};
  noggit::bool_toggle_property _draw_terrain = {true};
  noggit::bool_toggle_property _draw_wmo = {true};
  noggit::bool_toggle_property _draw_water = {true};
  noggit::bool_toggle_property _draw_wmo_doodads = {true};
  noggit::bool_toggle_property _draw_models = {true};
  noggit::bool_toggle_property _draw_model_animations = {false};
  noggit::bool_toggle_property _draw_hole_lines = {false};
  noggit::bool_toggle_property _draw_models_with_box = {false};
  noggit::bool_toggle_property _draw_fog = {true};
public:
  std::unordered_set<WMO*> _hidden_map_objects;
  std::unordered_set<Model*> _hidden_models;
  noggit::bool_toggle_property _draw_hidden_models = {false};
private:
  int _selected_area_id = -1;
  std::map<int, misc::random_color> _area_id_colors;

  float lastBrushUpdate;

  selection_result intersect_result(bool terrain_only);
  void doSelection(bool selectTerrainOnly);
  void update_cursor_pos();

  int mViewMode;

  void displayViewMode_2D();
  void displayViewMode_3D();

  void createGUI();

  float mTimespeed;

  void checkWaterSave();

  void ResetSelectedObjectRotation();
  void SnapSelectedObjectToGround();
  void DeleteSelectedObject();
  void changeZoneIDValue (int set);

  void insert_last_m2_from_wmv();
  void insert_last_wmo_from_wmv();

  QPointF _last_mouse_pos;
  float mh, mv, rh, rv;

  float moveratio = 0.1f;
  float rotratio = 0.2f;
  float keyx = 0, keyy = 0, keyz = 0, keyr = 0, keys = 0;

  float tool_settings_x;
  float tool_settings_y;

  bool MoveObj;

  math::vector_3d objMove;
  math::vector_3d objMoveOffset;
  math::vector_3d objRot;

  boost::optional<selection_type> lastSelected;

  bool TestSelection = false;

  bool  leftMouse = false;
  bool  leftClicked = false;
  bool  rightMouse = false;
  bool  painting = false;

  // Vars for the ground editing toggle mode store the status of some
  // view settings when the ground editing mode is switched on to
  // restore them if switch back again

  bool  alloff = true;
  bool  alloff_models = false;
  bool  alloff_doodads = false;
  bool  alloff_contour = false;
  bool  alloff_wmo = false;
  bool  alloff_detailselect = false;
  bool  alloff_fog = false;
  bool  alloff_terrain = false;

  editing_mode terrainMode = editing_mode::ground;
  editing_mode saveterrainMode = terrainMode;

  bool Saving = false;

  noggit::ui::toolbar* _toolbar;

  void prompt_exit();
  void prompt_save_current();

public:
  math::vector_4d cursor_color = math::vector_4d(1.0f, 1.0f, 1.0f, 1.0f);
  int cursor_type = 1;

  MapView ( math::degrees ah0
          , math::degrees av0
          , math::vector_3d camera_pos
          , noggit::ui::main_window*
          , World*
          );
  ~MapView();

  void tick (float dt);
  void display();

  void insert_object_at_selection_position (std::string);
  void selectModel(selection_type entry);

  void set_editing_mode (editing_mode);

private:
  enum Modifier
  {
    MOD_shift = 0x01,
    MOD_ctrl = 0x02,
    MOD_alt = 0x04,
    MOD_meta = 0x08,
    MOD_space = 0x10,
    MOD_none = 0x00,
  };
  struct HotKey
  {
    Qt::Key key;
    size_t modifiers;
    std::function<void()> function;
    std::function<bool()> condition;
    HotKey (Qt::Key k, size_t m, std::function<void()> f, std::function<bool()> c)
      : key (k), modifiers (m), function (f), condition (c) {}
  };

  std::forward_list<HotKey> hotkeys;

  void addHotkey(Qt::Key key, size_t modifiers, std::function<void()> function, std::function<bool()> condition = [] { return true; });
  bool handleHotkeys(QKeyEvent* e);

  QTime _startup_time;
  qreal _last_update = 0.f;

  QTimer _update_every_event_loop;

  virtual void initializeGL() override;
  virtual void paintGL() override;
  virtual void resizeGL (int w, int h) override;
  virtual void mouseMoveEvent (QMouseEvent*) override;
  virtual void mousePressEvent (QMouseEvent*) override;
  virtual void mouseReleaseEvent (QMouseEvent*) override;
  virtual void wheelEvent (QWheelEvent*) override;
  virtual void keyReleaseEvent (QKeyEvent*) override;
  virtual void keyPressEvent (QKeyEvent*) override;
  virtual void focusOutEvent (QFocusEvent*) override;

  noggit::ui::main_window* _main_window;

  math::vector_4d normalized_device_coords (int x, int y) const;
  float aspect_ratio() const;

  World* _world;

  float _tablet_pressure;
  bool _tablet_active = false;
#ifdef _WIN32
  HCTX hCtx = nullptr;
#endif
  void init_tablet();

  QLabel* _status_position;
  QLabel* _status_selection;
  QLabel* _status_area;
  QLabel* _status_time;

  noggit::bool_toggle_property _auto_selecting_mode = {true};

  noggit::bool_toggle_property _show_detail_info_window = {false};
  noggit::bool_toggle_property _show_minimap_window = {false};
  noggit::bool_toggle_property _show_cursor_switcher_window = {false};
  noggit::bool_toggle_property _show_keybindings_window = {false};
  noggit::bool_toggle_property _show_texture_palette_window = {false};

  noggit::ui::minimap_widget* _minimap;
  QDockWidget* _minimap_dock;

  void move_camera_with_auto_height (math::vector_3d const&);

  std::unique_ptr<noggit::ui::cursor_switcher> _cursor_switcher;
  std::unique_ptr<noggit::ui::help> _keybindings;

  noggit::ui::tileset_chooser* TexturePalette;
  noggit::ui::detail_infos* guidetailInfos;
  noggit::ui::zone_id_browser* ZoneIDBrowser;
  noggit::ui::texture_picker* TexturePicker;
  noggit::ui::water* guiWater;
  noggit::ui::current_texture* guiCurrentTexture;
  noggit::ui::object_editor* objectEditor;
  noggit::ui::flatten_blur_tool* flattenTool;
  noggit::ui::terrain_tool* terrainTool;
  QDockWidget* _terrain;
  noggit::ui::shader_tool* shaderTool;
  noggit::ui::texturing_tool* texturingTool;

  noggit::ui::exit_warning *escWarning;
  noggit::ui::water_save_warning *waterSaveWarning;
  noggit::ui::helper_models *HelperModels;
};
