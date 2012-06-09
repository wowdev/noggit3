// MapView.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MAPVIEW_H
#define MAPVIEW_H

// GL needs to be included before GLWidget.
#include <opengl/types.h>
#include <noggit/MapHeaders.h>

#include <QPoint>
#include <QGLWidget>
#include <QKeySequence>
#include <QTime>

class QAction;
class QMenu;
class QComboBox;
class QSlider;
class QSettings;
class QToolBar;
class QButtonGroup;
class QMenuBar;
class QLabel;

class World;
class nameEntry;

namespace noggit
{
  class blp_texture;

  namespace ui
  {
    class cursor_selector;
    class minimap_widget;
    class model_spawner;
    class EditorTemplate;
  }

  enum eViewMode
  {
    eViewMode_2D,
    eViewMode_3D,
  };

  class MapView : public QGLWidget
  {
    Q_OBJECT

  public:
    enum terrain_editing_modes
    {
      shaping = 0,
      smoothing = 1,
      texturing = 2,
      hole_setting = 3,
      area_id_setting = 4,
      impassable_flag_setting = 5,
    };

    //! \todo enum class with C++0x
    struct shaping_formula_type
    {
      enum formula
      {
        flat = 0,
        linear = 1,
        smooth = 2,
        polynomial = 3,
        trigonometric = 4,
        square = 5,
        shaping_formula_types,
      };
    };

    struct smoothing_formula_type
    {
      enum formula
      {
        flat = 0,
        linear = 1,
        smooth = 2,
        smoothing_formula_types,
      };
    };

    MapView ( World* world
            , qreal viewing_distance
            , float ah0 = -90.0f
            , float av0 = -30.0f
            , QGLWidget* shared = NULL
            , QWidget* parent = NULL
            );
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
    virtual void dropEvent (QDropEvent*);
    virtual void dragEnterEvent (QDragEnterEvent*);
    virtual void dragMoveEvent (QDragMoveEvent*);

  public slots:
    void set_terrain_editing_mode (int);
    void updateParent();

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
    void saveCata();
    void save_all();
    void save_minimap();
    void snap_selected_object_to_ground();
    void toggle_flag(bool);
    void toggle_auto_selecting (bool);
    void toggle_object_to_ground (bool);
    void toggle_copy_position_randomization (bool);
    void toggle_copy_rotation_randomization (bool);
    void toggle_copy_size_randomization (bool);
    void toggle_current_texture_visiblity (bool);
    void toggle_detail_info_window (bool);
    void toggle_interface();
    void toggle_lighting (bool);
    void toggle_minimap (bool);
    void toggle_terrain_mode_window();
    void toggle_terrain_texturing_mode();
    void toggle_tile_mode();
    void turn_around();
    void clear_heightmap();
    void move_heightmap();
    void set_area_id();
    void clear_all_models();
    void clear_texture();
    void show_texture_switcher();

    void shaping_formula (int);
    void shaping_formula (shaping_formula_type::formula);
    void shaping_radius (int);
    void shaping_speed (int);
    void shaping_radius (qreal);
    void shaping_speed (qreal);

    void smoothing_formula (int);
    void smoothing_formula (smoothing_formula_type::formula);
    void smoothing_radius (int);
    void smoothing_speed (int);
    void smoothing_radius (qreal);
    void smoothing_speed (qreal);

    void texturing_radius (int);
    void texturing_hardness (int);
    void texturing_pressure (int);
    void texturing_opacity (int);
    void texturing_radius (qreal);
    void texturing_hardness (qreal);
    void texturing_pressure (qreal);
    void texturing_opacity (qreal);

    void TEST_save_wdt();

  private:
    QAction* new_toggleable_action (const QString& text, const char* slot, bool default_value, const QKeySequence& shortcut = 0);
    QAction* new_flag_action (const QString& text, WorldFlags flag, const QKeySequence& shortcut = 0);
    QAction* new_action (const QString& text, const char* slot, const QKeySequence& shortcut = 0);
    QAction* new_action (const QString& text, QObject* receiver, const char* slot, const QKeySequence& shortcut = 0);

    void create_shaping_settings_widget();
    void create_smoothing_settings_widget();
    void create_paint_settings_widget();

    void createToolBar();

    void create_obsolete_gui();

    noggit::blp_texture *getSelectedTexture();

    const qreal& shaping_radius() const;
    const qreal& shaping_speed() const;
    const shaping_formula_type::formula& shaping_formula() const;

    const qreal& smoothing_radius() const;
    const qreal& smoothing_speed() const;
    const smoothing_formula_type::formula& smoothing_formula() const;

    const qreal& texturing_radius() const;
    const qreal& texturing_hardness() const;
    const qreal& texturing_pressure() const;
    const qreal& texturing_opacity() const;

    void draw_tile_mode_brush() const;
    QPointF tile_mode_brush_position() const;

    QTime _startup_time;
    qreal _last_update;

    float ah,av,moving,strafing,updown,mousedir,movespd;
    bool key_w;
    bool look;
    bool _GUIDisplayingEnabled;

    void doSelection( bool selectTerrainOnly );

    size_t flags;
    size_t backupFlags;
    int mViewMode;

    void displayViewMode_2D();
    void displayViewMode_3D();

    void createGUI();

    void setup_tile_mode_rendering() const;
    void setup_3d_rendering() const;
    void setup_3d_selection_rendering() const;
    void setup_2d_rendering() const;

    float mTimespeed;

    World* _world;

    QMenu* menu;

    ui::minimap_widget* _minimap;
    ui::model_spawner* _model_spawner;
    ui::cursor_selector* _cursor_selector;
    ui::EditorTemplate* editortemplate;

    QPoint _mouse_position;
    bool _is_currently_moving_object;

    bool _object_to_ground;

    bool _draw_lighting;

    float _fog_distance;

    bool _holding_left_mouse_button;
    bool _holding_right_mouse_button;

    terrain_editing_modes _current_terrain_editing_mode;
    terrain_editing_modes _terrain_editing_mode_before_2d;

    bool _save_to_minimap_on_next_drawing;

    //! \todo Use the brush class for these? Or simplify that somehow?

    QToolBar* toolBar;
    QButtonGroup* _toolbar_formula_radio_group;

    // shaping -----------
    qreal _shaping_radius;
    qreal _shaping_speed;
    QLabel* _shaping_radius_percent_label;
    QLabel* _shaping_speed_percent_label;
    shaping_formula_type::formula _shaping_formula;

    QComboBox* _shapingComboBox;
    QSlider* _shaping_radius_slider;
    QSlider* _shaping_speed_slider;
    QToolBar* _shaping_settings_widget;

    // smoothing -----------
    qreal _smoothing_radius;
    qreal _smoothing_speed;
    QLabel* _smoothing_radius_percent_label;
    QLabel* _smoothing_speed_percent_label;
    smoothing_formula_type::formula _smoothing_formula;

    QComboBox* _smoothingComboBox;
    QSlider* _smoothing_radius_slider;
    QSlider* _smoothing_speed_slider;
    QToolBar* _smoothing_settings_widget;

    // texturing -----------
    qreal _texturing_radius;
    qreal _texturing_hardness;
    qreal _texturing_pressure;
    qreal _texturing_opacity;

    QSlider* _texturing_radius_slider;
    QSlider* _texturing_hardness_slider;
    QSlider* _texturing_pressure_slider;
    QSlider* _texturing_opacity_slider;
    QToolBar* _texturing_settings_widget;
    QComboBox* _texturingComboBox;

    bool _automatically_update_terrain_selection;
    bool _copy_size_randomization;
    bool _copy_position_randomization;
    bool _copy_rotation_randomization;

    qreal _viewing_distance;
    qreal _tile_mode_zoom;

    bool _currently_holding_shift;
    bool _currently_holding_alt;
    bool _currently_holding_control;

    QSettings* _settings;
    nameEntry* _clipboard;
    int _selected_area_id;

    bool _invert_mouse_y_axis;
  };
}

#endif
