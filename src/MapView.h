#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QPoint>
#include <QGLWidget>
#include <QKeySequence>
#include <QTime>

class QAction;
class QMenu;

class UIFrame;
class World;
class minimap_widget;
class UIDoodadSpawner;

namespace ui
{
  class help_widget;
}

enum eViewMode
{
  eViewMode_Minimap,
  eViewMode_2D,
  eViewMode_3D
};

class MapView : public QGLWidget
{
  Q_OBJECT

public:
  MapView (World* world, float ah0 = -90.0f, float av0 = -30.0f, QGLWidget* shared = NULL, QWidget* parent = NULL);
  virtual ~MapView();

  virtual void tick( float t, float dt );
  virtual void display();

protected:
  virtual void initializeGL();
  virtual void paintGL();
  virtual void resizeGL (int w, int h);
  virtual void timerEvent (QTimerEvent*);
  virtual void keyPressEvent (QKeyEvent*);
  virtual void keyReleaseEvent (QKeyEvent*);
  virtual void mousePressEvent (QMouseEvent*);
  virtual void mouseReleaseEvent (QMouseEvent*);
  virtual void mouseMoveEvent (QMouseEvent*);

public slots:
  void resizewindow();

private slots:
  void add_bookmark();
  void copy_selected_object();
  void cycle_brush_type();
  void decrease_brush_size();
  void decrease_moving_speed();
  void decrease_time_speed();
  void delete_selected_object();
  void exit_to_menu();
  void export_heightmap();
  void import_heightmap();
  void increase_brush_size();
  void increase_moving_speed();
  void increase_time_speed();
  void invert_mouse_y_axis (bool);
  void paste_object();
  void reload_current_tile();
  void reset_selected_object_rotation();
  void save();
  void save_all();
  void save_minimap();
  void show_cursor_switcher();
  void show_map_chunk_settings (bool);
  void snap_selected_object_to_ground();
  void toggle_app_info (bool);
  void toggle_auto_selecting (bool);
  void toggle_contour_drawing (bool);
  void toggle_copy_position_randomization (bool);
  void toggle_copy_rotation_randomization (bool);
  void toggle_copy_size_randomization (bool);
  void toggle_current_texture_visiblity (bool);
  void toggle_detail_info_window (bool);
  void toggle_doodad_drawing (bool);
  void toggle_fog_drawing (bool);
  void toggle_hole_line_drawing (bool);
  void toggle_interface();
  void toggle_lighting (bool);
  void toggle_line_drawing (bool);
  void toggle_minimap (bool);
  void toggle_painting_mode (bool);
  void toggle_terrain_drawing (bool);
  void toggle_terrain_mode_window();
  void toggle_terrain_texturing_mode();
  void toggle_doodad_spawner();
  void toggle_tile_mode();
  void toggle_toolbar_visibility (bool);
  void toggle_water_drawing (bool);
  void toggle_wmo_doodad_drawing (bool);
  void toggle_wmo_drawing (bool);
  void turn_around();
  void clear_heightmap();
  void move_heightmap();
  void set_area_id();
  void clear_all_models();
  void clear_texture();
  void show_texture_switcher();

  void TEST_save_wdt();

private:
  QAction* new_toggleable_action (const QString& text, const char* slot, bool default_value, const QKeySequence& shortcut = 0);
  QAction* new_action (const QString& text, const char* slot, const QKeySequence& shortcut = 0);
  QAction* new_action (const QString& text, QObject* receiver, const char* slot, const QKeySequence& shortcut = 0);

  QTime _startup_time;
  qreal _last_update;

  float ah,av,moving,strafing,updown,mousedir,movespd;
  bool key_w;
  bool look;
  bool _GUIDisplayingEnabled;

  float lastBrushUpdate;

  void doSelection( bool selectTerrainOnly );

  int mViewMode;

  void displayViewMode_2D();
  void displayViewMode_3D();

  void displayGUIIfEnabled();

  void createGUI();

  float mTimespeed;

  World* _world;

  QPoint _last_drag_position;
  UIFrame* _last_clicked_item;

  minimap_widget* _minimap;
  UIDoodadSpawner* _doodad_spawner;
  ui::help_widget* _help_widget;
};


#endif
