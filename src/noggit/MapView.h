// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/bool_toggle_property.hpp>
#include <noggit/camera.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/unsigned_int_property.hpp>

#include <boost/optional.hpp>

#include <QtCore/QElapsedTimer>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QOpenGLWidget>
#include <QWidgetAction>

#include <forward_list>
#include <map>
#include <unordered_set>


class World;

namespace noggit
{
  class camera;
  namespace ui
  {
    class cursor_switcher;
    class detail_infos;
    class flatten_blur_tool;
    class help;
    class minimap_widget;
    class shader_tool;
    class terrain_tool;
    class texture_picker;
    class texturing_tool;
    class toolbar;
    class water;
    class zone_id_browser;
    class texture_palette_small;
    struct main_window;
    struct tileset_chooser;
  }
#ifdef NOGGIT_HAS_SCRIPTING
  namespace scripting
  {
    class scripting_tool;
  }
#endif
}

enum class save_mode
{
  current,
  changed,
  all
};

class MapView : public QOpenGLWidget
{
  Q_OBJECT
public:
  /// \todo getters?
  bool _mod_alt_down = false;
  bool _mod_ctrl_down = false;
  bool _mod_shift_down = false;
  bool _mod_space_down = false;
  bool _mod_num_down = false;
  bool  leftMouse = false;
  bool  leftClicked = false;
  bool  rightMouse = false;
  noggit::camera _camera;
  std::unique_ptr<World> _world;
private:

  float _2d_zoom = 1.f;
  float moving, strafing, updown, mousedir, turn, lookat;
  math::vector_3d _cursor_pos;

  bool look, freelook;
  bool ui_hidden = false;

  bool _camera_moved_since_last_draw = true;

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
  noggit::bool_toggle_property _draw_fog = {false};
public:
  noggit::bool_toggle_property _draw_hidden_models = {false};
private:
  int _selected_area_id = -1;
  std::map<int, misc::random_color> _area_id_colors;

  math::ray intersect_ray() const;
  selection_result intersect_result(bool terrain_only);
  void doSelection(bool selectTerrainOnly);
  void update_cursor_pos();

  display_mode _display_mode;

  math::matrix_4x4 model_view() const;
  math::matrix_4x4 projection() const;

  void draw_map();

  void createGUI();

  QWidgetAction* createTextSeparator(const QString& text);

  float mTimespeed;

  void ResetSelectedObjectRotation();
  void snap_selected_models_to_the_ground();
  void DeleteSelectedObject();
  void changeZoneIDValue (int set);

  QPointF _last_mouse_pos;
  float mh, mv, rh, rv;

  float keyx = 0, keyy = 0, keyz = 0, keyr = 0, keys = 0;

  bool MoveObj;
  float numpad_moveratio = 0.001f;

  math::vector_3d objMove;

  std::vector<selection_type> lastSelected;

  bool _rotation_editor_need_update = false;

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

  bool _uid_duplicate_warning_shown = false;
  bool _force_uid_check = false;
  bool _uid_fix_failed = false;
  void on_uid_fix_fail();

  uid_fix_mode _uid_fix;
  bool _from_bookmark;

  bool Saving = false;

  noggit::ui::toolbar* _toolbar;

  void save(save_mode mode);

  QSettings* _settings;

signals:
  void uid_fix_failed();
public slots:
  void on_exit_prompt();

public:
  math::vector_4d cursor_color;
  math::vector_4d shader_color;
  noggit::unsigned_int_property cursor_type;

  MapView ( math::degrees ah0
          , math::degrees av0
          , math::vector_3d camera_pos
          , noggit::ui::main_window*
          , std::unique_ptr<World>
          , uid_fix_mode uid_fix = uid_fix_mode::none
          , bool from_bookmark = false
          );
  ~MapView();

  void tick (float dt);
  void selectModel(std::string const& model);
  void change_selected_wmo_doodadset(int set);

  void set_editing_mode (editing_mode);

private:
  enum Modifier
  {
    MOD_shift = 0x01,
    MOD_ctrl = 0x02,
    MOD_alt = 0x04,
    MOD_meta = 0x08,
    MOD_space = 0x10,
    MOD_num = 0x20,
    MOD_none = 0x00
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

  QElapsedTimer _startup_time;
  qreal _last_update = 0.f;
  std::list<qreal> _last_frame_durations;

  float _last_fps_update = 0.f;

  QTimer _update_every_event_loop;

  virtual void tabletEvent(QTabletEvent* event) override;
  virtual void initializeGL() override;
  virtual void paintGL() override;
  virtual void resizeGL (int w, int h) override;
  virtual void enterEvent(QEvent*) override;
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

  float _tablet_pressure;
  bool _tablet_active = false;

  QLabel* _status_position;
  QLabel* _status_selection;
  QLabel* _status_area;
  QLabel* _status_time;
  QLabel* _status_fps;

  noggit::bool_toggle_property _locked_cursor_mode = {false};
  noggit::bool_toggle_property _move_model_to_cursor_position = {true};
  noggit::bool_toggle_property _snap_multi_selection_to_ground = {false};
  noggit::bool_toggle_property _rotate_along_ground = { true };
  noggit::bool_toggle_property _rotate_along_ground_smooth = { true };
  noggit::bool_toggle_property _rotate_along_ground_random = { false };

  noggit::bool_toggle_property _use_median_pivot_point = {true};
  noggit::bool_toggle_property _display_all_water_layers = {true};
  noggit::unsigned_int_property _displayed_water_layer = {0};
  noggit::object_paste_params _object_paste_params;

  noggit::bool_toggle_property _show_detail_info_window = {false};
  noggit::bool_toggle_property _show_minimap_window = {false};
  noggit::bool_toggle_property _show_minimap_borders = {true};
  noggit::bool_toggle_property _show_minimap_skies = {false};
  noggit::bool_toggle_property _show_cursor_switcher_window = {false};
  noggit::bool_toggle_property _show_keybindings_window = {false};
  noggit::bool_toggle_property _show_texture_palette_window = {false};
  noggit::bool_toggle_property _show_texture_palette_small_window = {false};

  noggit::ui::minimap_widget* _minimap;
  QDockWidget* _minimap_dock;
  QDockWidget* _texture_palette_dock;

  void move_camera_with_auto_height (math::vector_3d const&);

  void setToolPropertyWidgetVisibility(editing_mode mode);

  std::unique_ptr<noggit::ui::cursor_switcher> _cursor_switcher;
  noggit::ui::help* _keybindings;

  std::unordered_set<QDockWidget*> _tool_properties_docks;

  noggit::ui::tileset_chooser* TexturePalette;
  noggit::ui::detail_infos* guidetailInfos;
  noggit::ui::zone_id_browser* ZoneIDBrowser;
  noggit::ui::texture_palette_small* _texture_palette_small;
  QDockWidget* _areaid_editor_dock;
  noggit::ui::texture_picker* TexturePicker;
  noggit::ui::water* guiWater;
  QDockWidget* _water_editor_dock;
  noggit::ui::object_editor* objectEditor;
  QDockWidget* _object_editor_dock;
  noggit::ui::flatten_blur_tool* flattenTool;
  QDockWidget* _flatten_blur_dock;
  noggit::ui::terrain_tool* terrainTool;
  QDockWidget* _terrain_tool_dock;
  noggit::ui::shader_tool* shaderTool;
  QDockWidget* _vertex_shading_dock;
  noggit::ui::texturing_tool* texturingTool;
  QDockWidget* _texturing_dock;
#ifdef NOGGIT_HAS_SCRIPTING
  noggit::scripting::scripting_tool* scriptingTool;
  QDockWidget* _script_tool_dock;
#endif
};
