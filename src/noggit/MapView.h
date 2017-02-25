// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_4d.hpp>
#include <noggit/Selection.h>
#include <noggit/tool_enums.hpp>
#include <noggit/Misc.h>

#include <SDL.h>

#include <boost/optional.hpp>

#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtWidgets/QOpenGLWidget>

#include <forward_list>
#include <map>
#include <unordered_set>

class UIFrame;
class World;

class UICursorSwitcher;
class UIToggleGroup;
class UIMapViewGUI;

enum eViewMode
{
  eViewMode_Minimap,
  eViewMode_2D,
  eViewMode_3D
};

class WMO;
class Model;

class MapView : public QOpenGLWidget
{
private:
  bool _mod_alt_down = false;
  bool _mod_ctrl_down = false;
  bool _mod_shift_down = false;
  bool _mod_space_down = false;

  float _camera_ah;
  float _camera_av;
  math::vector_3d _camera_lookat;
  float _2d_zoom = 0.25f;
  float moving, strafing, updown, mousedir, movespd, turn, lookat;
  math::vector_3d _cursor_pos;
  bool key_w;
  bool look;
  bool _GUIDisplayingEnabled;

  bool _draw_contour = false;
  bool _draw_mfbo = false;
  bool _draw_wireframe = false;
  bool _draw_lines = false;
  bool _draw_terrain = true;
  bool _draw_wmo = true;
  bool _draw_water = true;
  bool _draw_wmo_doodads = true;
  bool _draw_models = true;
  bool _draw_model_animations = false;
  bool _draw_hole_lines = false;
  bool _draw_models_with_box = false;
public:
  std::unordered_set<WMO*> _hidden_map_objects;
  std::unordered_set<Model*> _hidden_models;
  bool _draw_hidden_models = false;
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

  void displayGUIIfEnabled();

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

  UICursorSwitcher* CursorSwitcher;

  bool Saving = false;

  UIFrame* LastClicked;

  UIMapViewGUI* mainGui;

  UIFrame* MapChunkWindow;
  UIToggleGroup * gFlagsToggleGroup;

  void prompt_exit();
  void prompt_save_current() const;

public:
  math::vector_4d cursor_color = math::vector_4d(1.0f, 1.0f, 1.0f, 1.0f);
  int cursor_type = 1;

  MapView(float ah0, float av0, math::vector_3d camera_pos, math::vector_3d camera_lookat);
  ~MapView();

  void tick (float dt);
  void display();

  void keyReleaseEvent (SDL_KeyboardEvent*);
  void keyPressEvent (SDL_KeyboardEvent*);
  void mouseReleaseEvent (SDL_MouseButtonEvent*);
  void mousePressEvent (SDL_MouseButtonEvent*);
  void resizewindow();

  void inserObjectFromExtern(int model);
  void selectModel(selection_type entry);

  void set_editing_mode (editing_mode);

private:
  enum Modifier
  {
    MOD_shift = 0x01,
    MOD_ctrl = 0x02,
    MOD_alt = 0x04,
    MOD_meta = 0x08,
    MOD_num = 0x10,
    MOD_caps = 0x20,
    MOD_mode = 0x40,
    MOD_none = 0x00,
  };
  struct HotKey
  {
    SDLKey key;
    size_t modifiers;
    std::function<void()> function;
    std::function<bool()> condition;
    HotKey (SDLKey k, size_t m, std::function<void()> f, std::function<bool()> c)
      : key (k), modifiers (m), function (f), condition (c) {}
  };

  std::forward_list<HotKey> hotkeys;

  void addHotkey(SDLKey key, size_t modifiers, std::function<void()> function, std::function<bool()> condition = [] { return true; });
  bool handleHotkeys(SDL_KeyboardEvent* e);

  QTime _startup_time;
  qreal _last_update;

  QTimer _update_every_event_loop;

  virtual void initializeGL() override;
  virtual void paintGL() override;
  virtual void resizeGL (int w, int h) override;
  virtual void closeEvent (QCloseEvent*) override;
  virtual void mouseMoveEvent (QMouseEvent*) override;
};
