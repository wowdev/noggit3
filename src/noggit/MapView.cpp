// MapView.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Beket <snipbeket@mail.ru>
// Bernd Lrwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/MapView.h>

#include <stdexcept>

#include <QMenu>
#include <QMenuBar>
#include <QKeyEvent>
#include <QSettings>
#include <QComboBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QSlider>
#include <QLabel>
#include <QToolBar>
#include <QVariant>

#include <math/bounded_nearest.h>

#include <helper/qt/signal_blocker.h>

#include <opengl/texture.h>

#include <noggit/Brush.h>
#include <noggit/MapChunk.h>
#include <noggit/WMOInstance.h>
#include <noggit/ModelManager.h>
#include <noggit/World.h>
#include <noggit/Log.h>
#include <noggit/ui/minimap_widget.h>
#include <noggit/ui/cursor_selector.h>
#include <noggit/ui/model_spawner.h>
#include <noggit/ui/editortemplate.h>
#include <noggit/ui/zoneid_widget.h>
#include <noggit/blp_texture.h>

//! \todo Replace all old gui elements with new classes / widgets.
#undef __OBSOLETE_GUI_ELEMENTS

Q_DECLARE_METATYPE (WorldFlags)

namespace noggit
{
  MapView::MapView ( World* world
                   , qreal viewing_distance
                   , float ah0
                   , float av0
                   , QGLWidget* shared
                   , QWidget* parent
                   )
    : QGLWidget (parent, shared)
    , _startup_time ()
    , _last_update (0.0)
    , ah (ah0)
    , av (av0)
    , _GUIDisplayingEnabled (true)
    , flags(TERRAIN | WMODOODAS | FOG | DOODADS | TERRAIN | DRAWWMO)
    , backupFlags(0)
    , mViewMode (eViewMode_3D)
    , mTimespeed (1.0f)
    , _world (world)
    , _minimap (new ui::minimap_widget (nullptr))
    , _model_spawner (new noggit::ui::model_spawner (nullptr,shared))
    , _cursor_selector (new ui::cursor_selector (nullptr))
    , _zoneid_widget( new ui::zoneid_widget (world,nullptr))
    , _is_currently_moving_object (false)
    , _object_to_ground (false)
    , _draw_lighting (true)
    , _fog_distance (777.0f)
    , _holding_left_mouse_button (false)
    , _holding_right_mouse_button (false)
    , _only_holding_right_mouse_button (false)
    , _current_terrain_editing_mode (shaping)
    , _terrain_editing_mode_before_2d (_current_terrain_editing_mode)
    , _save_to_minimap_on_next_drawing (false)
    , _shaping_radius (15.0)
    , _shaping_speed (1.0)
    , _shaping_formula (shaping_formula_type::smooth)
    , _shapingComboBox (nullptr)
    , _shaping_radius_slider (nullptr)
    , _shaping_speed_slider (nullptr)
    , _shaping_settings_widget (nullptr)
    , _smoothing_radius (15.0)
    , _smoothing_speed (1.0)
    , _smoothing_formula (smoothing_formula_type::smooth)
    , _smoothingComboBox (nullptr)
    , _smoothing_radius_slider (nullptr)
    , _smoothing_speed_slider (nullptr)
    , _smoothing_settings_widget (nullptr)
    , _texturing_radius (15.0)
    , _texturing_hardness (0.9f)
    , _texturing_pressure (0.9f)
    , _texturing_opacity (1.0f)
    , _texturing_radius_slider (nullptr)
    , _texturing_hardness_slider (nullptr)
    , _texturing_pressure_slider (nullptr)
    , _texturing_opacity_slider (nullptr)
    , _texturing_settings_widget (nullptr)
    , _texturingComboBox (nullptr)
    , _automatically_update_terrain_selection (true)
    , _copy_size_randomization (false)
    , _copy_position_randomization (false)
    , _copy_rotation_randomization (false)
    , _viewing_distance (viewing_distance)
    , _tile_mode_zoom (0.25)
    , _currently_holding_shift (false)
    , _currently_holding_alt (false)
    , _currently_holding_control (false)
    , _settings (new QSettings (this))
    , _clipboard (boost::none)
    , _invert_mouse_y_axis (false)
    , moving (0.0f)
    , strafing (0.0f)
    , updown (0.0f)
    , movespd (66.6f)
  {
    setMinimumSize (500, 500);
    setMaximumHeight (2000);
    setAcceptDrops (true);
    setFocusPolicy (Qt::StrongFocus);
    setMouseTracking (true);
    //flags |= _draw_terrain ? TERRAIN : 0;
    createGUI();

    //! \todo Dynamic?
    startTimer (40);
    //! \todo QTimerEvent->time_since_startup or something?
    _startup_time.start();

  }

  namespace
  {
    float mh,mv,rh,rv;
    float keyx,keyy,keyz,keyr,keys;
  }

  void MapView::createGUI()
  {
    _minimap->world (_world);
    _minimap->draw_skies (true);
    _minimap->draw_camera (true);
    _minimap->draw_boundaries (true);
    _minimap->hide();

    _model_spawner->hide();

    create_shaping_settings_widget();
    create_smoothing_settings_widget();
    create_paint_settings_widget();

    createToolBar();

    create_obsolete_gui();

  #define NEW_ACTION(__NAME__, __TEXT, __SLOT, __KEYS) QAction* __NAME__ (new_action (__TEXT, __SLOT, __KEYS));
  #define NEW_ACTION_OTHER(__NAME__, __TEXT, __RECEIVER, __SLOT, __KEYS) QAction* __NAME__ (new_action (__TEXT, __RECEIVER, __SLOT, __KEYS));
  #define NEW_TOGGLE_ACTION(__NAME__, __TEXT, __SLOT, __KEYS, __DEFAULT) QAction* __NAME__ (new_toggleable_action (__TEXT, __SLOT, __DEFAULT, __KEYS));
  #define NEW_FLAG_ACTION(__NAME__, __TEXT, __FLAG, __KEYS) QAction* __NAME__ (new_flag_action (__TEXT, __FLAG, __KEYS));

    NEW_ACTION (save_current_tile, tr ("Save current tile"), SLOT (save()), Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    NEW_ACTION (save_modified_tiles, tr ("Save all modified tiles"), SLOT (save_all()), QKeySequence::Save);
    NEW_ACTION (reload_tile, tr ("Reload current tile"), SLOT (reload_current_tile()), Qt::SHIFT + Qt::Key_J);

    NEW_ACTION (bookmark, tr ("Add bookmark for this location"), SLOT (add_bookmark()), Qt::Key_F5);

    NEW_ACTION (export_heightmap, tr ("Export heightmap"), SLOT (export_heightmap()), 0);
    NEW_ACTION (import_heightmap, tr ("Import heightmap"), SLOT (import_heightmap()), 0);

    NEW_ACTION (to_menu, tr ("Return to menu"), SLOT (exit_to_menu()), Qt::Key_Escape);


    NEW_ACTION (copy_object, tr ("Copy object"), SLOT (copy_selected_object()), QKeySequence::Copy);
    NEW_ACTION (paste_object, tr ("Paste object"), SLOT (paste_object()), QKeySequence::Paste);
    NEW_ACTION (delete_object, tr ("Delete object"), SLOT (delete_selected_object()), QKeySequence::Delete);
    NEW_TOGGLE_ACTION (object_to_ground, tr ("Auto-object to Ground"), SLOT (toggle_object_to_ground (bool)), 0, _object_to_ground);
    //delete_object->setShortcuts (QList<QKeySequence>() << QKeySequence::Delete << Qt::Key_Backspace);

    NEW_ACTION (reset_rotation, tr ("Reset object's rotation"), SLOT (reset_selected_object_rotation()), Qt::CTRL + Qt::Key_R);
    NEW_ACTION (snap_object_to_ground, tr ("Snap object to ground"), SLOT (snap_selected_object_to_ground()), Qt::Key_PageDown);


    NEW_TOGGLE_ACTION (current_texture, tr ("Selected texture"), SLOT (toggle_current_texture_visiblity (bool)), 0, false);
    NEW_TOGGLE_ACTION (toggle_minimap, tr ("Show minimap"), SLOT (toggle_minimap (bool)), Qt::Key_M, false);
    NEW_TOGGLE_ACTION (detail_infos, tr ("Object information"), SLOT (toggle_detail_info_window (bool)), Qt::Key_F8, false);


    NEW_FLAG_ACTION (doodad_drawing, tr ("Draw doodads"), DOODADS, Qt::Key_F1);
    NEW_FLAG_ACTION (wmo_doodad_drawing, tr ("Draw doodads inside of WMOs"), WMODOODAS, Qt::Key_F2);
    NEW_FLAG_ACTION (terrain_drawing, tr ("Draw terrain"), TERRAIN, Qt::Key_F3);
    NEW_FLAG_ACTION (water_drawing, tr ("Draw water"), WATER, Qt::Key_F4);
    NEW_FLAG_ACTION (wmo_drawing, tr ("Draw WMOs"), DRAWWMO, Qt::Key_F6);
    NEW_FLAG_ACTION (line_drawing, tr ("Draw lines"), LINES, Qt::Key_F7);
    NEW_FLAG_ACTION (hole_line_drawing, tr ("Draw lines for holes"), HOLELINES, Qt::SHIFT + Qt::Key_F7);
    //! \todo on OSX this shows up as "8" in menu and does not react to the keybinding.
    NEW_FLAG_ACTION (contour_drawing, tr ("Draw contours"), HEIGHTCONTOUR, Qt::Key_F9);
    NEW_FLAG_ACTION (fog_drawing, tr ("Draw fog"), FOG, Qt::Key_F);

    //! \todo is unused atm
    NEW_TOGGLE_ACTION (toggle_lighting, tr ("Enable Lighting"), SLOT (toggle_lighting (bool)), Qt::Key_L, _draw_lighting);

    NEW_ACTION (turn_around, tr ("Turn camera 180 degrees"), SLOT (turn_around()), Qt::Key_R);

    NEW_ACTION_OTHER (cursor_selector, tr ("Choose selection cursor"), _cursor_selector, SLOT (show()), Qt::ALT + Qt::Key_C);
    NEW_TOGGLE_ACTION (invert_mouse_y_axis, tr ("Invert mouse y-axis"), SLOT (invert_mouse_y_axis (bool)), Qt::Key_I, _invert_mouse_y_axis);
    NEW_TOGGLE_ACTION (auto_selection, tr ("Automatic selection"), SLOT (toggle_auto_selecting (bool)), Qt::SHIFT + Qt::Key_F4, false);

    NEW_TOGGLE_ACTION (rotation_randomization, tr ("Randomized rotation when copying"), SLOT (toggle_copy_rotation_randomization (bool)), 0, false);
    NEW_TOGGLE_ACTION (position_randomization, tr ("Randomized position when copying"), SLOT (toggle_copy_position_randomization (bool)), 0, false);
    NEW_TOGGLE_ACTION (size_randomization, tr ("Randomized size when copying"), SLOT (toggle_copy_size_randomization (bool)), 0, false);

    NEW_ACTION (decrease_time_speed, tr ("Decrease time speed"), SLOT (decrease_time_speed()), Qt::Key_B);
    NEW_ACTION (increase_time_speed, tr ("Increase time speed"), SLOT (increase_time_speed()), Qt::Key_N);
    NEW_ACTION (decrease_moving_speed, tr ("Decrease movement speed"), SLOT (decrease_moving_speed()), Qt::Key_O);
    NEW_ACTION (increase_moving_speed, tr ("Increase movement speed"), SLOT (increase_moving_speed()), Qt::Key_P);

    NEW_ACTION (save_wdt, tr ("Save WDT"), SLOT (TEST_save_wdt()), 0);
    NEW_ACTION (save_minimap, tr ("Save minimap as raw files"), SLOT (save_minimap()), Qt::Key_P + Qt::SHIFT + Qt::CTRL);
    NEW_ACTION_OTHER (model_spawner, tr ("Add object to map"), _model_spawner, SLOT (show()), Qt::Key_T);
    NEW_ACTION (save_current_tile_cata, tr ("Save current tile (Cata tests)"), SLOT (saveCata()), 0);
    NEW_ACTION_OTHER (area_id_browser, tr ("Area Id Browser"), _zoneid_widget, SLOT (show()), 0);

  #undef NEW_ACTION
  #undef NEW_ACTION_OTHER
  #undef NEW_TOGGLE_ACTION

    QMenu* file_menu (new QMenu (tr ("File")));
    _menus.emplace_back (file_menu);
    file_menu->addAction (save_current_tile);
    file_menu->addAction (save_modified_tiles);
    file_menu->addAction (reload_tile);
    file_menu->addSeparator();
    file_menu->addAction (bookmark);
    file_menu->addSeparator();
    file_menu->addAction (export_heightmap);
    file_menu->addAction (import_heightmap);
    file_menu->addSeparator();
    file_menu->addAction (to_menu);

    QMenu* edit_menu (new QMenu (tr ("Edit")));
    _menus.emplace_back (edit_menu);
    edit_menu->addAction (copy_object);
    edit_menu->addAction (paste_object);
    edit_menu->addAction (delete_object);
    edit_menu->addAction (object_to_ground);
    edit_menu->addSeparator();
    edit_menu->addAction (reset_rotation);
    edit_menu->addAction (snap_object_to_ground);

    /*
    QMenu* assist_menu (new QMenu (tr ("Assist")));
    _menus.emplace_back (assist_menu);

    QMenu* insertion_menu (assist_menu->addMenu (tr ("Insert helper model")));

    assist_menu->addAction (tr ("all from MV", InsertObject, 0  );
    assist_menu->addAction (tr ("last M2 from MV", InsertObject, 14  );
    assist_menu->addAction (tr ("last WMO from MV", InsertObject, 15  );
    assist_menu->addAction (tr ("from Text File", InsertObject, 1  );
    insertion_menu->addAction (tr ("Human scale", InsertObject, 2  );
    insertion_menu->addAction (tr ("Cube 50", InsertObject, 3  );
    insertion_menu->addAction (tr ("Cube 100", InsertObject, 4  );
    insertion_menu->addAction (tr ("Cube 250", InsertObject, 5  );
    insertion_menu->addAction (tr ("Cube 500", InsertObject, 6  );
    insertion_menu->addAction (tr ("Cube 1000", InsertObject, 7  );
    insertion_menu->addAction (tr ("Disc 50", InsertObject, 8  );
    insertion_menu->addAction (tr ("Disc 200", InsertObject, 9  );
    insertion_menu->addAction (tr ("Disc 777", InsertObject, 10  );
    insertion_menu->addAction (tr ("Sphere 50", InsertObject, 11  );
    insertion_menu->addAction (tr ("Sphere 200", InsertObject, 12  );
    insertion_menu->addAction (tr ("Sphere 777", InsertObject, 13  );
    assist_menu->addSeparator();
    assist_menu->addAction (tr ("Set Area ID", set_area_id, 0  );
    assist_menu->addAction (tr ("Clear height map", clear_heightmap, 0  );
    assist_menu->addAction (tr ("Move to position", move_heightmap, 0  );
    assist_menu->addAction (tr ("Clear texture", clear_texture, 0  );
    assist_menu->addAction (tr ("Clear models", clear_all_models, 0  );
    assist_menu->addAction (tr ("Switch texture", show_texture_switcher, 0  );
  */
    QMenu* view_menu (new QMenu (tr ("View")));
    _menus.emplace_back (view_menu);
    view_menu->addAction (current_texture);
    view_menu->addAction (toggle_minimap);
    view_menu->addAction (detail_infos);
    view_menu->addSeparator();
    view_menu->addAction (doodad_drawing);
    view_menu->addAction (wmo_doodad_drawing);
    view_menu->addAction (terrain_drawing);
    view_menu->addAction (water_drawing);
    view_menu->addAction (wmo_drawing);
    view_menu->addAction (line_drawing);
    view_menu->addAction (hole_line_drawing);
    view_menu->addAction (contour_drawing);
    view_menu->addAction (fog_drawing);
    view_menu->addAction (toggle_lighting);

    QMenu* settings_menu (new QMenu (tr ("Settings")));
    _menus.emplace_back (settings_menu);
    settings_menu->addAction (cursor_selector);
    settings_menu->addSeparator();
    settings_menu->addAction (rotation_randomization);
    settings_menu->addAction (position_randomization);
    settings_menu->addAction (size_randomization);
    settings_menu->addSeparator();
    settings_menu->addAction (auto_selection);
    settings_menu->addAction (invert_mouse_y_axis);
    settings_menu->addSeparator();
    settings_menu->addAction (decrease_time_speed);
    settings_menu->addAction (increase_time_speed);
    settings_menu->addAction (decrease_moving_speed);
    settings_menu->addAction (increase_moving_speed);

    QMenu* debug_menu (new QMenu (tr ("Testing and Debugging")));
    _menus.emplace_back (debug_menu);
    debug_menu->addAction (save_wdt);
    debug_menu->addAction (model_spawner);
    debug_menu->addAction (save_minimap);
    debug_menu->addAction (save_current_tile_cata);
    debug_menu->addAction (area_id_browser);

    QMenu* useless_menu (debug_menu->addMenu (tr ("Stuff that should only be on keys")));
    useless_menu->addAction (turn_around);

    set_terrain_editing_mode(_current_terrain_editing_mode);
  }

  QAction* MapView::new_action (const QString& text, const char* slot, const QKeySequence& shortcut)
  {
    QAction* action (new QAction (text, this));
    connect (action, SIGNAL (triggered()), slot);
    if (shortcut != QKeySequence (0))
    {
      action->setShortcuts (QList<QKeySequence>() << shortcut);
    }
    return action;
  }

  QAction* MapView::new_action (const QString& text, QObject* receiver, const char* slot, const QKeySequence& shortcut)
  {
    QAction* action (new QAction (text, this));
    receiver->connect (action, SIGNAL (triggered()), slot);
    if (shortcut != QKeySequence (0))
    {
      action->setShortcuts (QList<QKeySequence>() << shortcut);
    }
    return action;
  }

  QAction* MapView::new_toggleable_action (const QString& text, const char* slot, bool default_value, const QKeySequence& shortcut)
  {
    QAction* action (new QAction (text, this));
    connect (action, SIGNAL (toggled (bool)), slot);
    action->setCheckable (true);
    action->setChecked (default_value);
    if (shortcut != QKeySequence (0))
    {
      action->setShortcuts (QList<QKeySequence>() << shortcut);
    }
    return action;
  }

  QAction *MapView::new_flag_action(const QString &text, WorldFlags flag, const QKeySequence &shortcut)
  {
    QAction* action (new QAction (text, this));
    connect (action, SIGNAL(toggled(bool)), SLOT(toggle_flag(bool)) );
    action->setCheckable (true);
    action->setData(flag);
    action->setChecked (flags & flag);
    if (shortcut != QKeySequence (0))
    {
      action->setShortcuts (QList<QKeySequence>() << shortcut);
    }
    return action;
  }

  void MapView::dropEvent (QDropEvent* event)
  {
    const QMimeData* mime (event->mimeData());
    if (mime->hasFormat (ui::model_spawner::mime_type()))
    {
      event->accept();
      const QString path (mime->data (ui::model_spawner::mime_type()));

      if (path.endsWith (".m2"))
      {
        _world->addM2 (path.toStdString(), _world->_exact_terrain_selection_position);
      }
      else if (path.endsWith (".wmo"))
      {
        _world->addWMO (path.toStdString(), _world->_exact_terrain_selection_position);
      }
    }
    else
    {
      QWidget::dropEvent (event);
    }
  }

  void MapView::dragEnterEvent (QDragEnterEvent* event)
  {
    if (event->mimeData()->hasFormat (ui::model_spawner::mime_type()))
    {
      event->acceptProposedAction();
      _mouse_position = event->pos();
      //! \todo Already render the model half transparent instead of a cursor.
    }
    else
    {
      QWidget::dragEnterEvent (event);
    }
  }

  void MapView::dragMoveEvent (QDragMoveEvent* event)
  {
    _mouse_position = event->pos();
    event->accept();
  }

  void MapView::toggle_current_texture_visiblity (bool value)
  {
#ifdef __OBSOLETE_GUI_ELEMENTS
    mainGui->SelectedTexture->hidden (!value);
#endif
  }

  void MapView::toggle_copy_size_randomization (bool value)
  {
    _copy_size_randomization = value;
  }
  void MapView::toggle_copy_position_randomization (bool value)
  {
    _copy_position_randomization = value;
  }
  void MapView::toggle_copy_rotation_randomization (bool value)
  {
    _copy_rotation_randomization = value;
  }

  void MapView::timerEvent (QTimerEvent*)
  {
    this->makeCurrent();
    const qreal now (_startup_time.elapsed() / 1000.0);

    tick (now - _last_update);
    updateGL();

    _last_update = now;
  }

  void MapView::initializeGL()
  {
    this->makeCurrent();
    qglClearColor (Qt::black);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);

    glEnable (GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT0);
    glEnable (GL_MULTISAMPLE);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
  }

  void MapView::paintGL()
  {
    this->makeCurrent();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    display();
  }

  void MapView::resizeGL (int width, int height)
  {
    glViewport (0.0f, 0.0f, width, height);
  }

  MapView::~MapView()
  {
    delete _world;
    _world = nullptr;
  }

  void MapView::maybe_move_selection_depending_on_weird_global_variables()
  {
    if (!_selection || !keyx || !keyy || !keyz || !keyr || !keys)
    {
      //! \note As this code is called every tick instead of only when needed, we need to prevent it from crashing with selection::is_chunk (*_selection).
      return;
    }

    qreal keypad_object_move_ratio (0.1);
    if(_currently_holding_control && _currently_holding_shift)
    {
      keypad_object_move_ratio *= 0.5;
    }
    else if(_currently_holding_shift)
    {
      keypad_object_move_ratio *= 2.0;
    }
    else if(_currently_holding_control)
    {
      keypad_object_move_ratio *= 3.0;
    }

    selection::move ( ::math::vector_3d (keyx, keyy, keyz)
                    * keypad_object_move_ratio
                    , *_selection
                    );
    selection::rotate_y ( keyr * keypad_object_move_ratio * 2.0
                        , *_selection
                        );
    selection::scale ( keys * keypad_object_move_ratio / 50.0
                     , *_selection
                     );
  }

  void MapView::tick(float dt )
  {
    ::math::vector_3d dir( 1.0f, 0.0f, 0.0f );
    ::math::vector_3d dirUp( 1.0f, 0.0f, 0.0f );
    ::math::vector_3d dirRight( 0.0f, 0.0f, 1.0f );
    ::math::rotate (0.0f, 0.0f, &dir.x(), &dir.y(), av);
    ::math::rotate (0.0f, 0.0f, &dir.x(), &dir.z(), ah);

    if(_currently_holding_shift)
    {
      dirUp.x (0.0f);
      dirUp.y (1.0f);
      dirRight = ::math::vector_3d (0.0f, 0.0f, 0.0f);
    }
    else if(_currently_holding_control)
    {
      dirUp.x (0.0f);
      dirUp.y (1.0f);
      ::math::rotate (0.0f, 0.0f, &dirUp.x(), &dirUp.y(), av);
      ::math::rotate (0.0f, 0.0f, &dirRight.x(), &dirRight.y(), av);
      ::math::rotate (0.0f, 0.0f, &dirUp.x(), &dirUp.z(), ah);
      ::math::rotate (0.0f, 0.0f, &dirRight.x(), &dirRight.z(), ah);
    }
    else
    {
      ::math::rotate (0.0f, 0.0f, &dirUp.x(), &dirUp.z(), ah);
      ::math::rotate (0.0f, 0.0f, &dirRight.x(), &dirRight.z(), ah);
    }

    maybe_move_selection_depending_on_weird_global_variables();

    if (_selection)
    {
      //! \todo   Alternatively   automatically   align  it   to   the
      //! terrain. Also try to move it where the mouse points.
      if (_is_currently_moving_object)
      {
        if (_currently_holding_alt)
        {
          selection::scale (powf (2.0f, mv * 4.0f), *_selection);
        }
        else
        {
          //! \note The  commented out code  here was executed  when a
          //! model was  selected. I am  unsure, if it is  relevant as
          //! ObjPos.x()  is  the  only   thing  used.   Also,  it  is
          //! overwritten    before   being   used.    ObjPos.x()   is
          //! weird_constant now.
          // ObjPos = Selection->data.model->pos - _world->camera;
          // ::math::rotate (0.0f, 0.0f, &ObjPos.x(), &ObjPos.y(), av);
          // ::math::rotate (0.0f, 0.0f, &ObjPos.x(), &ObjPos.z(), ah);
          // ObjPos.x (fabs (ObjPos.x()));
          static const float weird_constant (80.0f);
          const ::math::vector_3d offset ( mv * dirUp * weird_constant
                                         - mh * dirRight * weird_constant
                                         );
          selection::move (offset, *_selection);

          if(_object_to_ground)
          {
            snap_selected_object_to_ground();
          }
        }

        mh = 0.0f;
        mv = 0.0f;
      }

      if (_only_holding_right_mouse_button)
      {
        if (_currently_holding_control)
        {
          selection::rotate_x (rh + rv, *_selection);
        }
        else if (_currently_holding_shift)
        {
          selection::rotate_y (rh + rv, *_selection);
        }
        else if (_currently_holding_alt)
        {
          selection::rotate_z (rh + rv, *_selection);
        }

        rh = 0.0f;
        rv = 0.0f;
      }

      if (_holding_left_mouse_button && selection::is_chunk (*_selection))
      {
        const ::math::vector_3d& position
          (_world->_exact_terrain_selection_position);

        switch(_current_terrain_editing_mode)
        {
        case shaping:
          if(mViewMode == eViewMode_3D)
          {
            if(_currently_holding_shift)
            {
              _world->changeTerrain ( position.x()
                                    , position.z()
                                    , 7.5f * dt * shaping_speed()
                                    , shaping_radius()
                                    , shaping_formula()
                                    );
            }
            else if(_currently_holding_control)
            {
              _world->changeTerrain ( position.x()
                                    , position.z()
                                    , -7.5f * dt * shaping_speed()
                                    , shaping_radius()
                                    , shaping_formula()
                                    );
            }
          }
          break;

        case smoothing:
          if(mViewMode == eViewMode_3D)
          {
            if(_currently_holding_shift)
            {
              _world->flattenTerrain ( position.x()
                                     , position.z()
                                     , position.y()
                                     , pow( 0.2f, dt ) * smoothing_speed()
                                     , smoothing_radius()
                                     , smoothing_formula()
                                     );
            }
            else if(_currently_holding_control)
            {
              _world->blurTerrain ( position.x()
                                  , position.z()
                                  , pow( 0.2f, dt ) * smoothing_speed()
                                  , std::min( smoothing_radius(), 30.0 )
                                  , smoothing_formula()
                                  );
            }
          }
          break;

        case texturing:
          {
            const QPointF brush_position ( mViewMode == eViewMode_3D
                                         ? QPointF ( position.x()
                                                   , position.z()
                                                   )
                                         : tile_mode_brush_position()
                                         + QPointF ( _world->camera.x()
                                                   , _world->camera.z()
                                                   )
                                         );

            if( mViewMode == eViewMode_3D )
            {
              if ( _currently_holding_shift
                && _currently_holding_control
                 )
              {
                _world->eraseTextures (brush_position.x(), brush_position.y());
              }
              else if (_currently_holding_control)
              {
#ifdef __OBSOLETE_GUI_ELEMENTS
                mainGui->TexturePicker->getTextures
                  (_world->GetCurrentSelection());
#endif
              }
              else if (_currently_holding_shift)
              {
                _world->paintTexture ( brush_position.x()
                                     , brush_position.y()
                                     , brush (_texturing_radius, _texturing_hardness)
                                     , _texturing_opacity * 255.0f
                                     , 1.0f - powf ( 1.0f - (float)_texturing_pressure
                                                   , (float)dt * 10.0f
                                                   )
                                     , getSelectedTexture()
                                     );
              }
            }
            else
            {
              _world->paintTexture ( brush_position.x()
                                   , brush_position.y()
                                   , brush (_texturing_radius, _texturing_hardness)
                                   , _texturing_opacity * 255.0f
                                   , 1.0f - powf ( 1.0f - (float)_texturing_pressure
                                                 , (float)dt * 10.0f
                                                 )
                                   , getSelectedTexture()
                                   );
            }
          }
        break;

        case hole_setting:
          if(mViewMode == eViewMode_3D)
          {
            if (_currently_holding_shift)
            {
              const ::math::vector_3d selection_position
                (selection::position (*_selection));

              _world->removeHole ( selection_position.x()
                                 , selection_position.z()
                                 , _currently_holding_alt
                                 );
            }
            else if(_currently_holding_control)
            {
              _world->addHole ( position.x()
                              , position.z()
                              , position.y()
                              , _currently_holding_alt
                              );
            }
          }
        break;

        case area_id_setting:
          if(mViewMode == eViewMode_3D)
          {
            if(_currently_holding_shift)
            {
              _world->setAreaID (_selected_area_id, position);
            }
            else if(_currently_holding_control)
            {
              _selected_area_id = selection::area_id (*_selection);
#ifdef __OBSOLETE_GUI_ELEMENTS
              mainGui->ZoneIDBrowser->setZoneID(_selected_area_id);
#endif
              _zoneid_widget->setZoneID(_selected_area_id);

			 // in zeile 761 aus int(_world->GetCurrentSelection()->data.mapchunk->areaID) _selected_area_id machen.

              _zoneid_widget->setMapID();
            }
          }
        break;

        case impassable_flag_setting:
          if(mViewMode == eViewMode_3D)
          {
            if(_currently_holding_shift)
            {
              _world->setFlag( true, position.x(), position.z() );
            }
            else if(_currently_holding_control)
            {
              _world->setFlag( false, position.x(), position.z() );
            }
          }
        break;
        }
      }
    }

    move_camera (dt, dir);

    _world->advance_times (dt, mTimespeed);
    _world->tick (dt);

#ifdef __OBSOLETE_GUI_ELEMENTS
    if (!_map_chunk_properties_window->hidden()
      && selection::is_chunk (*_selection)
       )
    {
      UITexturingGUI::setChunkWindow
        ( _world->GetCurrentSelection()->data.mapchunk );
    }
#endif

    //! \todo This should only be done when actually needed. (on movement and camera changes as well as modifying an adt)
    _minimap->update();
  }

  void MapView::move_camera (const float& dt, const ::math::vector_3d& dir)
  {
    if(mViewMode != eViewMode_2D)
    {
      if(moving)
        _world->camera += dir * dt * movespd * moving;
      if(strafing)
      {
        ::math::vector_3d right = dir % ::math::vector_3d( 0.0f, 1.0f ,0.0f );
        right.normalize();
        _world->camera += right * dt * movespd * strafing;
      }
      if(updown)
        _world->camera += ::math::vector_3d( 0.0f, dt * movespd * updown, 0.0f );

      _world->lookat = _world->camera + dir;
    }
    else
    {
      if(moving)
        _world->camera.z ( _world->camera.z()
                         - dt * movespd * moving / (_tile_mode_zoom * 1.5f)
                         );
      if(strafing)
        _world->camera.x ( _world->camera.x()
                         + dt * movespd * strafing / (_tile_mode_zoom * 1.5f)
                         );
      if(updown)
        _tile_mode_zoom *= pow( 2.0f, dt * updown * 4.0f );

      _tile_mode_zoom = qBound (0.1, _tile_mode_zoom, 2.0);
    }
  }

  void MapView::setup_tile_mode_rendering() const
  {
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    const qreal ratio (width() / qreal (height()));
    glOrtho (-2.0f * ratio, 2.0f * ratio, 2.0f, -2.0f, -100.0f, 300.0f );

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
  }

  void MapView::setup_2d_rendering() const
  {
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    glOrtho (0.0f, width(), height(), 0.0f, -1.0f, 1.0f);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
  }

  static const qreal nearclip (1.0);
  static const qreal fov (45.0);
  void MapView::setup_3d_rendering() const
  {
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    const qreal ratio (width() / qreal (height()));
    gluPerspective (fov, ratio, 1.0f, _viewing_distance);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
  }

  void MapView::setup_3d_selection_rendering() const
  {
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    GLint viewport[4] = {0, 0, width(), height()};
    gluPickMatrix ( _mouse_position.x()
                  , height() - _mouse_position.y()
                  , 7
                  , 7
                  , viewport
                  );

    const qreal ratio (width() / qreal (height()));
    gluPerspective (fov, ratio, 1.0f, _viewing_distance);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
  }

  void MapView::doSelection( bool selectTerrainOnly )
  {
    setup_3d_selection_rendering();

    if (selectTerrainOnly)
    {
      _selection = _world->drawSelection ( flags & ( ~DOODADS
                                                   & ~DRAWWMO
                                                   & ~WMODOODAS
                                                   )
                                         );
    }
    else
    {
      _selection = _world->drawSelection (flags);
    }
  }

  QPointF MapView::tile_mode_brush_position() const
  {
    const QPointF mouse_pos ( _mouse_position.x() / qreal (width()) - 0.5
                            , _mouse_position.y() / qreal (height()) - 0.5
                            );

    static const qreal arbitrary_constant_due_to_viewport (CHUNKSIZE * 4.0);

    return QPointF ( arbitrary_constant_due_to_viewport
                   * width() / qreal (height())
                   * mouse_pos.x()
                   / _tile_mode_zoom
                   , arbitrary_constant_due_to_viewport
                   * mouse_pos.y()
                   / _tile_mode_zoom
                   );
  }

  void MapView::draw_tile_mode_brush() const
  {
    const qreal brush_radius (_texturing_radius);
    const qreal brush_diameter (brush_radius * 2.0);
    const QPointF brush_position (tile_mode_brush_position());

    glPushMatrix();

    glColor4f (1.0f, 1.0f, 1.0f, 0.5f);
    opengl::texture::enable_texture (0);
    brush (_texturing_radius, _texturing_hardness).getTexture()->bind();

    glScalef (_tile_mode_zoom / CHUNKSIZE, _tile_mode_zoom / CHUNKSIZE, 1.0f);
    glTranslatef ( brush_position.x() - _texturing_radius
                 , brush_position.y() - _texturing_radius
                 , 0.0f
                 );

    glBegin (GL_QUADS);
    glTexCoord2f (0.0f, 0.0f);
    glVertex3f (0.0f, brush_diameter, 0.0f);
    glTexCoord2f (1.0f, 0.0f);
    glVertex3f (brush_diameter, brush_diameter, 0.0f);
    glTexCoord2f (1.0f, 1.0f);
    glVertex3f (brush_diameter, 0.0f, 0.0f);
    glTexCoord2f (0.0f, 1.0f);
    glVertex3f (0.0f, 0.0f, 0.0f);
    glEnd();
    glPopMatrix();
  }

  void MapView::displayViewMode_2D()
  {
    setup_tile_mode_rendering();
    _world->drawTileMode ( flags & LINES, width() / qreal (height())
                         , _tile_mode_zoom
                         );
    draw_tile_mode_brush();
  }

  void MapView::displayViewMode_3D()
  {
    const bool update_selection (_automatically_update_terrain_selection
                              && ( !_selection
                                || selection::is_chunk (*_selection)
                                 )
                                );

    if (update_selection)
    {
      doSelection (true);
    }

    setup_3d_rendering();

    float brush_radius (0.3f);

    if (_current_terrain_editing_mode == shaping)
      brush_radius = shaping_radius();
    else if (_current_terrain_editing_mode == smoothing)
      brush_radius = smoothing_radius();
    else if (_current_terrain_editing_mode == texturing)
      brush_radius = texturing_radius();

    _world->draw ( flags
                 , brush_radius
                 , brush_radius
                 , _mouse_position
                 , _fog_distance
                 , _selection
                 );
  }

  void MapView::display()
  {
    //! \todo  Get this out or do it somehow else. This is ugly and is a senseless if each draw.
    if (_save_to_minimap_on_next_drawing)
    {
      setup_tile_mode_rendering();
      _world->saveMap();
      _save_to_minimap_on_next_drawing = false;
    }

    switch( mViewMode )
    {
    case eViewMode_2D:
      displayViewMode_2D();
      break;

    case eViewMode_3D:
      displayViewMode_3D();
      break;
    }
  }

  void MapView::keyPressEvent (QKeyEvent* event)
  {
    if (event->key() == Qt::Key_Shift)
      _currently_holding_shift = true;

    if (event->key() == Qt::Key_Alt)
      _currently_holding_alt = true;

    if (event->key() == Qt::Key_Control)
      _currently_holding_control = true;

    // movement
    if (event->key() == Qt::Key_W)
    {
        key_w = true;
        moving = 1.0f;
    }

    if (event->key() == Qt::Key_S)
      moving = -1.0f;

    if (event->key() == Qt::Key_A)
      strafing = -1.0f;

    if (event->key() == Qt::Key_D)
      strafing = 1.0f;

    if (event->key() == Qt::Key_E)
      updown = -1.0f;

    if (event->key() == Qt::Key_Q)
      updown = 1.0f;

    //! \todo FUCK.
  /*
    // position correction with num pad
    if (event->key() == Qt::Key_KP8 )
      keyx = -1;

    if (event->key() == Qt::Key_KP2 )
      keyx = 1;

    if (event->key() == Qt::Key_KP6 )
      keyz = -1;

    if (event->key() == Qt::Key_KP4 )
      keyz = 1;

    if (event->key() == Qt::Key_KP1 )
      keyy = -1;

    if (event->key() == Qt::Key_KP3 )
      keyy = 1;

    if (event->key() == Qt::Key_KP7 )
      keyr = 1;

    if (event->key() == Qt::Key_KP9 )
      keyr = -1;
      */

    if (event->key() == Qt::Key_X)
      toggle_terrain_mode_window();

  //  NEW_ACTION (snap_object_to_ground, tr ("Snap object to ground"), SLOT (snap_selected_object_to_ground()), Qt::Key_PageDown);
  //  NEW_TOGGLE_ACTION (rotation_randomization, tr ("Randomized rotation when copying"), SLOT (toggle_copy_rotation_randomization (bool)), 0, false);


    if (event->key() == Qt::Key_C)
    {
      _settings->setValue ( "cursor/type"
                          , (_settings->value ("cursor/type").toInt() + 1) % 4
                          );
      _settings->sync();
    }

    if (event->key() == Qt::Key_U)
      toggle_tile_mode();

    if (event->key() == Qt::Key_Y)
      cycle_brush_type();

    if (event->key() == Qt::Key_Tab)
      toggle_interface();


    if (event->key() == Qt::Key_F1 && event->modifiers() & Qt::ShiftModifier)
      toggle_terrain_texturing_mode();

    static const float fog_distance_step (60.0f);

    //! \todo All  these are not  exclusive but additive which  is not
    //! only bad but wrong. (alt+shift will still increase brush size)
    if (event->key() == Qt::Key_Plus)
    {
      if( event->modifiers() & Qt::AltModifier )
      {
        increase_brush_size();
      }
      else if( event->modifiers() & Qt::ShiftModifier)
      {
        _fog_distance += fog_distance_step;
      }
      else //change selected model size
      {
        keys=1;
      }
    }

    if (event->key() == Qt::Key_Minus)
    {
      if (event->modifiers() & Qt::AltModifier)
      {
        decrease_brush_size();
      }
      else if( event->modifiers() & Qt::ShiftModifier)
      {
        _fog_distance -= fog_distance_step;
      }
      else //change selected model size
      {
        keys=-1;
      }
    }

    // doodads set
    if( event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9 )
    {
      const int key (event->key() - Qt::Key_0);
      if (event->modifiers() & Qt::ShiftModifier && key >= 1 && key <= 4)
      {
        static const float speeds[4] = {15.0f, 50.0f, 300.0f, 1000.0f};
        movespd = speeds[key - 1];
      }
      else if (_selection && selection::is_wmo (*_selection))
      {
        selection::set_doodad_set (key, *_selection);
      }
      else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_6)
      {
        set_terrain_editing_mode (terrain_editing_modes (key - 1));
      }
    }
  }

  void MapView::toggle_detail_info_window (bool value)
  {
#ifdef __OBSOLETE_GUI_ELEMENTS
    mainGui->guidetailInfos->setVisible (value);
#endif
  }

  void MapView::toggle_terrain_mode_window()
  {
#ifdef __OBSOLETE_GUI_ELEMENTS
    if(_current_terrain_editing_mode == texturing)
      view_texture_palette( 0, 0 );
    else if(_current_terrain_editing_mode == area_id_setting)
      mainGui->ZoneIDBrowser->toggleVisibility();
#endif
  }

  void MapView::invert_mouse_y_axis (bool value)
  {
    _invert_mouse_y_axis = value;
  }

  void MapView::delete_selected_object()
  {
    if (!_selection)
    {
      return;
    }

    selection::remove_from_world (_world, *_selection);
  }

  void MapView::paste_object()
  {
    if (!_selection || !_clipboard)
    {
      return;
    }

    //! \todo Visitor to call addM2 or addWMO directly.
    _world->addModel ( **_clipboard
                     , selection::position (*_selection)
                     , _copy_size_randomization
                     , _copy_position_randomization
                     , _copy_rotation_randomization
                     );
  }

  void MapView::copy_selected_object()
  {
    if (!_selection)
    {
      return;
    }

    _clipboard = selection::name_entry (*_selection);
  }

  void MapView::increase_moving_speed()
  {
    movespd *= 2.0f;
  }

  void MapView::decrease_moving_speed()
  {
    movespd *= 0.5f;
  }

  void MapView::save_minimap()
  {
    //! \todo This needs to be actually done here, not deferred to next display().
    _save_to_minimap_on_next_drawing = true;
  }

  void MapView::turn_around()
  {
    ah += 180.0f;
  }



  void MapView::reset_selected_object_rotation()
  {
    if (!_selection)
    {
      return;
    }

    selection::reset_rotation (*_selection);
  }

  void MapView::snap_selected_object_to_ground()
  {
    if (selection::is_chunk (*_selection))
    {
      throw std::runtime_error ( "Can not snap chunks to ground "
                                 "as it already is the ground, duh."
                               );
    }

    ::math::vector_3d position (selection::position (*_selection));
    boost::optional<float> height ( _world->get_height ( position.x()
                                                       , position.y()
                                                       )
                                  );

    if (!height)
    {
      throw std::runtime_error ( "Tried snapping to ground while there is "
                                 "no ground below the object."
                               );
    }

    selection::move ( ::math::vector_3d (0.0f, *height, 0.0f) - position
                    , *_selection
                    );
  }

  void MapView::toggle_interface()
  {
    _GUIDisplayingEnabled = !_GUIDisplayingEnabled;
  }

  static const qreal time_speed_step_length (0.01);
  void MapView::increase_time_speed()
  {
    mTimespeed += time_speed_step_length;
  }
  void MapView::decrease_time_speed()
  {
    mTimespeed = std::max (qreal (0), mTimespeed - time_speed_step_length);
  }

  void MapView::toggle_terrain_texturing_mode()
  {
    //! \todo This is BAD global state! Use a stack<settings_type> as member.
    static bool in_texturing_mode (false);

    if( !in_texturing_mode )
    {
      backupFlags = flags;
      flags = TERRAIN | HEIGHTCONTOUR;
    }
    else
    {
      flags = backupFlags;
    }
    in_texturing_mode = !in_texturing_mode;
  }

  void MapView::toggle_auto_selecting (bool value)
  {
    _automatically_update_terrain_selection = value;
  }

  void MapView::toggle_object_to_ground (bool value)
  {
    _object_to_ground = value;
  }

  /// --- Drawing toggles --------------------------------------------------------

  void MapView::toggle_flag (bool value)
  {
    QAction *action = (QAction *)sender();
    if(value)
        flags |= action->data().toInt();
    else
        flags &= ~action->data().toInt();
  }

  void MapView::toggle_lighting (bool value)
  {
    _draw_lighting = value;
  }


  //! \todo these should be symetrical, so maybe combine.
  void MapView::increase_brush_size()
  {
    switch(_current_terrain_editing_mode)
    {
    case shaping:
      shaping_radius (shaping_radius() + 0.01);
      break;
    case smoothing:
      smoothing_radius (smoothing_radius() + 0.01);
      break;
    case texturing:
      texturing_radius (texturing_radius() + 0.01);
      break;
    default:
      break;
    }
  }

  void MapView::decrease_brush_size()
  {
    switch( _current_terrain_editing_mode )
    {
    case shaping:
      shaping_radius (shaping_radius() - 0.01);
      break;
    case smoothing:
      smoothing_radius (smoothing_radius() - 0.01);
      break;
    case texturing:
      texturing_radius (texturing_radius() - 0.01);
      break;
    default:
      break;
    }
  }

  void MapView::toggle_minimap (bool value)
  {
    _minimap->setVisible (value);
  }

  void MapView::save_all()
  {
    _world->saveChanged();
  }

  void MapView::save()
  {
    _world->saveTile( static_cast<int>( _world->camera.x() ) / TILESIZE, static_cast<int>( _world->camera.z() ) / TILESIZE );
  }

  void MapView::saveCata()
  {
    _world->saveTileCata( static_cast<int>( _world->camera.x() ) / TILESIZE, static_cast<int>( _world->camera.z() ) / TILESIZE );
  }

  void MapView::reload_current_tile()
  {
    _world->reloadTile( static_cast<int>( _world->camera.x() ) / TILESIZE, static_cast<int>( _world->camera.z() ) / TILESIZE );
  }

  void MapView::exit_to_menu()
  {
    close();
  }

  void MapView::cycle_brush_type()
  {
    // toogle between smooth / flat / linear
    switch( _current_terrain_editing_mode )
    {
    case shaping:
      shaping_formula ((shaping_formula() + 1) % shaping_formula_type::shaping_formula_types);
      break;

    case smoothing:
      smoothing_formula ((smoothing_formula() + 1) % smoothing_formula_type::smoothing_formula_types);
      break;

    default:
      break;
    }
  }

  void MapView::toggle_tile_mode()
  {
    if( mViewMode == eViewMode_2D )
    {
      mViewMode = eViewMode_3D;
      set_terrain_editing_mode (_terrain_editing_mode_before_2d);
    }
    else
    {
      mViewMode = eViewMode_2D;
      _terrain_editing_mode_before_2d = _current_terrain_editing_mode;
      set_terrain_editing_mode (texturing);
    }
  }

  struct BookmarkEntry
  {
    int map_id;
    int area_id;
    ::math::vector_3d position;
    float rotation;
    float tilt;
  };

  void MapView::add_bookmark()
  {
    QSettings settings;

    //! \todo This MUST be easier and not needing to read and insert everything.
    QList<BookmarkEntry> bookmarks;

    int bookmarks_count (settings.beginReadArray ("bookmarks"));
    for (int i (0); i < bookmarks_count; ++i)
    {
      settings.setArrayIndex (i);

      BookmarkEntry b;
      b.map_id = settings.value ("map_id").toInt();
      b.position.x (settings.value ("camera/position/x").toFloat());
      b.position.y (settings.value ("camera/position/y").toFloat());
      b.position.z (settings.value ("camera/position/z").toFloat());
      b.rotation = settings.value ("camera/rotation").toFloat();
      b.tilt = settings.value ("camera/tilt").toFloat();
      b.area_id = settings.value ("area_id").toInt();

      bookmarks.append (b);
    }
    settings.endArray();

    BookmarkEntry new_bookmark;
    new_bookmark.map_id = _world->getMapID();
    new_bookmark.area_id = _world->getAreaID();
    new_bookmark.position = ::math::vector_3d (_world->camera.x(), _world->camera.y(), _world->camera.z());
    new_bookmark.rotation = ah;
    new_bookmark.tilt = av;

    bookmarks.append (new_bookmark);

    settings.beginWriteArray ("bookmarks");
    for (int i (0); i < bookmarks.size(); ++i)
    {
      settings.setArrayIndex (i);

      settings.setValue ("map_id", bookmarks[i].map_id);
      settings.setValue ("camera/position/x", bookmarks[i].position.x());
      settings.setValue ("camera/position/y", bookmarks[i].position.y());
      settings.setValue ("camera/position/z", bookmarks[i].position.z());
      settings.setValue ("camera/rotation", bookmarks[i].rotation);
      settings.setValue ("camera/tilt", bookmarks[i].tilt);
      settings.setValue ("area_id", bookmarks[i].area_id);
    }
    settings.endArray();

    //! \todo Signal the change of settings somehow, so Menu can update.
  }

  void MapView::keyReleaseEvent (QKeyEvent* event)
  {
    if (event->key() == Qt::Key_Shift)
    {
      _currently_holding_shift = false;
    }

    if (event->key() == Qt::Key_Alt)
    {
      _currently_holding_alt = false;
    }

    if (event->key() == Qt::Key_Control)
    {
      _currently_holding_control = false;
    }

    // movement
    if (event->key() == Qt::Key_W)
    {
      key_w = false;
      if( !(_holding_left_mouse_button && _holding_right_mouse_button) && moving > 0.0f) moving = 0.0f;
    }

    if (event->key() == Qt::Key_S && moving < 0.0f )
      moving = 0.0f;

    if (event->key() == Qt::Key_D && strafing > 0.0f )
      strafing = 0.0f;

    if (event->key() == Qt::Key_A && strafing < 0.0f )
      strafing = 0.0f;

    if (event->key() == Qt::Key_Q && updown > 0.0f )
      updown = 0.0f;

    if (event->key() == Qt::Key_E && updown < 0.0f )
      updown = 0.0f;

    //! \todo FUCK.
  /*
    if (event->key() == Qt::Key_KP8 )
      keyx = 0;

    if (event->key() == Qt::Key_KP2 )
      keyx = 0;

    if (event->key() == Qt::Key_KP6 )
      keyz = 0;

    if (event->key() == Qt::Key_KP4 )
      keyz = 0;

    if (event->key() == Qt::Key_KP1 )
      keyy = 0;

    if (event->key() == Qt::Key_KP3 )
      keyy = 0;

    if (event->key() == Qt::Key_KP7 )
      keyr = 0;

    if (event->key() == Qt::Key_KP9 )
      keyr = 0;

    if (event->key() == Qt::Key_KP_MINUS || e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
      keys = 0;
      */
  }

  void MapView::mousePressEvent (QMouseEvent* event)
  {
    if (event->button() == Qt::LeftButton)
    {
      _holding_left_mouse_button = true;
    }
    if (event->button() == Qt::RightButton)
    {
      _holding_right_mouse_button = true;
    }

    if (event->button() == Qt::LeftButton && _selection)
    {
      _is_currently_moving_object = true;
    }

    if (_holding_left_mouse_button && _holding_right_mouse_button)
    {
      moving = 1.0f;
    }
    else if (_holding_left_mouse_button && mViewMode == eViewMode_3D)
    {
      doSelection (false);
    }
    else if (_holding_right_mouse_button)
    {
      _only_holding_right_mouse_button = true;
    }
  }

  void MapView::mouseReleaseEvent (QMouseEvent* event)
  {
    if (event->button() == Qt::LeftButton)
    {
      _holding_left_mouse_button = false;
      _is_currently_moving_object = false;

      if (!key_w && moving > 0.0f)
        moving = 0.0f;

      if (mViewMode == eViewMode_2D)
      {
        strafing = 0;
        moving = 0;
      }
    }
    if (event->button() == Qt::RightButton)
    {
      _holding_right_mouse_button = false;
      _only_holding_right_mouse_button = false;

      if (!key_w && moving > 0.0f)
        moving = 0.0f;

      if (mViewMode == eViewMode_2D)
        updown = 0;
    }
  }

  void MapView::mouseMoveEvent (QMouseEvent* event)
  {
    static const float XSENS (15.0f);
    static const float YSENS (15.0f);

    const QPoint relative_move (event->pos() - _mouse_position);

    if ( _only_holding_right_mouse_button
      && event->modifiers() == Qt::NoModifier
       )
    {
      ah += relative_move.x() / XSENS;
      av += (_invert_mouse_y_axis ? 1.0 : -1.0) * relative_move.y() / YSENS;

      av = qBound (-80.0f, av, 80.0f);
    }

    //! \todo Actually move and rotate objects here instead of ::tick().
    if(_is_currently_moving_object)
    {
      // TODO find better formula ( it have still some missing x, y)
      // Very slowed when snap_object_to_ground
      mh = -relative_move.x() / XSENS / 5.0f;
      mv = -relative_move.y() / YSENS / 5.0f;
    }
    else
    {
      mh = 0.0f;
      mv = 0.0f;
    }

    if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier))
    {
      rh = relative_move.x() / XSENS * 5.0f;
      rv = relative_move.y() / YSENS * 5.0f;
    }

    if (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::AltModifier)
    {
      switch( _current_terrain_editing_mode )
      {
      case shaping:
        shaping_radius (shaping_radius() + relative_move.x() / XSENS);
        connect (_shaping_radius_slider, SIGNAL(valueChanged (int)), _shaping_radius_percent_label, SLOT(setNum (int)));
        break;

      case smoothing:
        smoothing_radius (smoothing_radius() + relative_move.x() / XSENS);
        connect (_smoothing_radius_slider, SIGNAL(valueChanged (int)), _smoothing_radius_percent_label, SLOT(setNum (int)));
        break;

      case texturing:
        texturing_radius (texturing_radius() + relative_move.x() / XSENS);
        break;
    default:
      break;
      }
    }

    if ( mViewMode == eViewMode_2D
      && event->buttons() & Qt::LeftButton
      && event->modifiers() == Qt::ControlModifier
       )
    {
      strafing = ((relative_move.x() / XSENS) / -1.0f) * 5.0f;
      moving = (relative_move.y() / YSENS) * 5.0f;
    }

    if ( mViewMode == eViewMode_2D
      && event->buttons() & Qt::RightButton
      && event->modifiers() == Qt::ControlModifier
       )
    {
      updown = (relative_move.y() / YSENS);
    }

    _mouse_position = event->pos();
  }

  void MapView::set_terrain_editing_mode (int mode)
  {
    _current_terrain_editing_mode = (terrain_editing_modes)mode;

    flags &= ~HOLELINES;
    flags &= ~NOCURSOR;
    flags &= ~AREAID;
    flags &= ~MARKIMPASSABLE;

    _shaping_settings_widget->hide();
    _smoothing_settings_widget->hide();
    _texturing_settings_widget->hide();

    switch (mode)
    {
    case shaping:
      _shaping_settings_widget->show();
      break;

    case smoothing:
      _smoothing_settings_widget->show();
      break;

    case texturing:
      _texturing_settings_widget->show();
      break;

    case hole_setting:
      flags |= HOLELINES | NOCURSOR;
      break;

    case area_id_setting:
      flags |= AREAID;
      break;

    case impassable_flag_setting:
      flags |= MARKIMPASSABLE;
      break;

    default:
      break;
    }

#ifdef __OBSOLETE_GUI_ELEMENTS
    if (mainGui && mainGui->guiToolbar)
      mainGui->guiToolbar->set_icon_visual (_current_terrain_editing_mode);
#endif
  }

  void MapView::import_heightmap()
  {
    //! \todo Ask _world to import.
  }
  void MapView::export_heightmap()
  {
    //! \todo Ask _world to export.
  }

  void MapView::TEST_save_wdt()
  {
    _world->saveWDT();
  }

  static int tile_below_camera (const float& position)
  {
    return ::math::bounded_nearest<int>
      ((position - (TILESIZE / 2)) / TILESIZE);
  }

  void MapView::move_heightmap()
  {
    //! \note Pre-condition: _selection

    const float height_delta ( _world->camera.y()
                             - selection::position (*_selection).y()
                             );
    _world->moveHeight ( tile_below_camera (_world->camera.x())
                       , tile_below_camera (_world->camera.z())
                       , height_delta
                       );
  }

  void MapView::clear_heightmap()
  {
    _world->clearHeight ( tile_below_camera (_world->camera.x())
                        , tile_below_camera (_world->camera.z())
                        );
  }

  void MapView::set_area_id()
  {
    //! \todo This should not be called via menu requiring to have been set somewhere else prior to calling this. THIS IS UTTER BULLSHIT.
    if (_selected_area_id)
    {
      _world->setAreaID ( _selected_area_id
                        , tile_below_camera (_world->camera.x())
                        , tile_below_camera (_world->camera.z())
                        );
    }
  }

  void MapView::clear_all_models()
  {
    _world->clearAllModelsOnADT ( tile_below_camera (_world->camera.x())
                                , tile_below_camera (_world->camera.z())
                                );
  }

  void MapView::clear_texture()
  {
    _world->eraseTextures ( tile_below_camera (_world->camera.x())
                          , tile_below_camera (_world->camera.z())
                          );
  }

  noggit::scoped_blp_texture_reference MapView::getSelectedTexture()
  {
      return {_texturingComboBox->itemData(_texturingComboBox->currentIndex()).toString().toStdString()};
  }


  // --- brush settings constants ------------------------------------------------------------------

  namespace detail
  {
    struct slider_settings
    {
      qreal minimum;
      qreal maximum;
      qreal scale;

      slider_settings (const qreal& minimum_, const qreal& maximum_, const qreal& scale_)
        : minimum (minimum_), maximum (maximum_), scale (scale_)
      { }
    };
  }

  static const detail::slider_settings shaping_radius_constants (0.1, 533.3, 100.0);
  static const detail::slider_settings shaping_speed_constants (0.0, 10.0, 10.0);

  static const detail::slider_settings smoothing_radius_constants (0.1, 533.3, 100.0);
  static const detail::slider_settings smoothing_speed_constants (0.0, 10.0, 10.0);

  static const detail::slider_settings texturing_radius_constants (0.1, 533.3, 100.0);
  static const detail::slider_settings texturing_pressure_constants (0.01, 1.0, 100.0);
  static const detail::slider_settings texturing_hardness_constants (0.0, 1.0, 100.0);
  static const detail::slider_settings texturing_opacity_constants (0.0, 1.0, 100.0);

  // --- brush settings getter / setters -----------------------------------------------------------

  #define BRUSH_FORMULA_METHODS(BRUSHNAME)                                                        \
    void MapView::BRUSHNAME ## _formula (int id)                                                  \
    {                                                                                             \
      BRUSHNAME ## _formula (BRUSHNAME ## _formula_type::formula (id));                           \
    }                                                                                             \
    void MapView::BRUSHNAME ## _formula (BRUSHNAME ## _formula_type::formula id)                  \
    {                                                                                             \
      _ ## BRUSHNAME ## _formula = id;                                                            \
      helper::qt::signal_blocker block (_ ## BRUSHNAME ## _formula_radio_group);                  \
      _ ## BRUSHNAME ## _formula_radio_group->button (id)->click();                               \
    }                                                                                             \
    const MapView::BRUSHNAME ## _formula_type::formula& MapView::BRUSHNAME ## _formula() const    \
    {                                                                                             \
      return _ ## BRUSHNAME ## _formula;                                                          \
    }

  #define BRUSH_FORMULA_METHODS_COMBOBOX(BRUSHNAME)                                               \
    void MapView::BRUSHNAME ## _formula (int id)                                                  \
    {                                                                                             \
      BRUSHNAME ## _formula (BRUSHNAME ## _formula_type::formula (id));                           \
    }                                                                                             \
    void MapView::BRUSHNAME ## _formula (BRUSHNAME ## _formula_type::formula id)                  \
    {                                                                                             \
      _ ## BRUSHNAME ## _formula = id;                                                            \
      helper::qt::signal_blocker block (_ ## BRUSHNAME ## ComboBox);                              \
      _ ## BRUSHNAME ## ComboBox->setCurrentIndex (id);                                           \
    }                                                                                             \
    const MapView::BRUSHNAME ## _formula_type::formula& MapView::BRUSHNAME ## _formula() const    \
    {                                                                                             \
      return _ ## BRUSHNAME ## _formula;                                                          \
    }

  #define BRUSH_SLIDER_METHODS(BRUSHNAME, VARIABLENAME)                                           \
    void MapView::BRUSHNAME ## _ ## VARIABLENAME (int value)                                      \
    {                                                                                             \
      BRUSHNAME ## _ ## VARIABLENAME (value / BRUSHNAME ## _ ## VARIABLENAME ## _constants.scale);\
    }                                                                                             \
    void MapView::BRUSHNAME ## _ ## VARIABLENAME (qreal value)                                    \
    {                                                                                             \
      _ ## BRUSHNAME ## _ ## VARIABLENAME                                                         \
        = qBound ( BRUSHNAME ## _ ## VARIABLENAME ## _constants.minimum                           \
                 , value                                                                          \
                 , BRUSHNAME ## _ ## VARIABLENAME ## _constants.maximum                           \
                 );                                                                               \
      helper::qt::signal_blocker block (_ ## BRUSHNAME ## _ ## VARIABLENAME ## _slider);          \
      _ ## BRUSHNAME ## _ ## VARIABLENAME ## _slider->                                            \
        setValue ( BRUSHNAME ## _ ## VARIABLENAME ()                                              \
                 * BRUSHNAME ## _ ## VARIABLENAME ## _constants.scale                             \
                 );                                                                               \
    }                                                                                             \
    const qreal& MapView::BRUSHNAME ## _ ## VARIABLENAME() const                                  \
    {                                                                                             \
      return _ ## BRUSHNAME ## _ ## VARIABLENAME;                                                 \
    }

  BRUSH_FORMULA_METHODS_COMBOBOX (shaping);
  BRUSH_SLIDER_METHODS (shaping, radius);
  BRUSH_SLIDER_METHODS (shaping, speed);

  BRUSH_FORMULA_METHODS_COMBOBOX (smoothing);
  BRUSH_SLIDER_METHODS (smoothing, radius);
  BRUSH_SLIDER_METHODS (smoothing, speed);

  BRUSH_SLIDER_METHODS (texturing, radius);
  BRUSH_SLIDER_METHODS (texturing, hardness);
  BRUSH_SLIDER_METHODS (texturing, pressure);
  BRUSH_SLIDER_METHODS (texturing, opacity);

  #undef BRUSH_FORMULA_METHODS
  #undef BRUSH_SLIDER_METHODS

  // --- brush settings widgets --------------------------------------------------------------------

  void MapView::create_shaping_settings_widget()
  {
    delete _shaping_settings_widget;

    _shaping_settings_widget = new QToolBar;

    _shapingComboBox = new QComboBox (this);
    _shapingComboBox->setStyleSheet("margin-right: 10px;padding-left:2px;");

    _shapingComboBox->addItem (tr ("Flat"), shaping_formula_type::flat);
    _shapingComboBox->addItem (tr ("Linear"), shaping_formula_type::linear);
    _shapingComboBox->addItem (tr ("Smooth"), shaping_formula_type::smooth);
    _shapingComboBox->addItem (tr ("Polynomial"), shaping_formula_type::polynomial);
    _shapingComboBox->addItem (tr ("Trigonometric"), shaping_formula_type::trigonometric);
    _shapingComboBox->addItem (tr ("Square"), shaping_formula_type::square);

    connect (_shapingComboBox, SIGNAL(currentIndexChanged(int)), SLOT(shaping_formula(int)));

    _shaping_settings_widget->addWidget (_shapingComboBox);

    _shaping_radius_slider = new QSlider (Qt::Horizontal, _shaping_settings_widget);
    _shaping_radius_slider->setMinimum (shaping_radius_constants.minimum * shaping_radius_constants.scale);
    _shaping_radius_slider->setMaximum (shaping_radius_constants.maximum * shaping_radius_constants.scale);
    _shaping_radius_slider->setMaximumWidth (200);
    connect (_shaping_radius_slider, SIGNAL (valueChanged (int)), SLOT (shaping_radius (int)));

    _shaping_speed_slider = new QSlider (Qt::Horizontal, _shaping_settings_widget);
    _shaping_speed_slider->setMinimum (shaping_speed_constants.minimum * shaping_speed_constants.scale);
    _shaping_speed_slider->setMaximum (shaping_speed_constants.maximum * shaping_speed_constants.scale);
    _shaping_speed_slider->setMaximumWidth (200);
    connect (_shaping_speed_slider, SIGNAL (valueChanged (int)), SLOT (shaping_speed (int)));

    QLabel* radius_label (new QLabel (tr ("Brush &radius"), _shaping_settings_widget));
    QLabel* speed_label (new QLabel (tr ("Shaping &speed"), _shaping_settings_widget));
    _shaping_radius_percent_label = new QLabel (_shaping_settings_widget);
    _shaping_speed_percent_label = new QLabel (_shaping_settings_widget);

    connect (_shaping_radius_slider, SIGNAL (valueChanged (int)), _shaping_radius_percent_label, SLOT (setNum (int)));
    connect (_shaping_speed_slider,  SIGNAL (valueChanged (int)), _shaping_speed_percent_label, SLOT (setNum (int)));

    radius_label->setBuddy (_shaping_radius_slider);
    speed_label->setBuddy (_shaping_speed_slider);
    _shaping_radius_percent_label->setBuddy (_shaping_radius_slider);
    _shaping_speed_percent_label->setBuddy (_shaping_speed_slider);

    speed_label->setStyleSheet("margin-left:10px");

    _shaping_settings_widget->addWidget (radius_label);
    _shaping_settings_widget->addWidget (_shaping_radius_slider);
    _shaping_settings_widget->addWidget (_shaping_radius_percent_label);
    _shaping_settings_widget->addWidget (speed_label);
    _shaping_settings_widget->addWidget (_shaping_speed_slider);
    _shaping_settings_widget->addWidget (_shaping_speed_percent_label);

    //! \note Looks funny, but sets the UI to the default position.
    shaping_radius (shaping_radius());
    shaping_speed (shaping_speed());
    shaping_formula (shaping_formula());
  }

  void MapView::create_smoothing_settings_widget()
  {
    delete _smoothing_settings_widget;

    _smoothing_settings_widget = new QToolBar (nullptr);

    _smoothingComboBox = new QComboBox (_smoothing_settings_widget);
    _smoothingComboBox->setStyleSheet("margin-right: 10px;padding-left:2px;");

    _smoothingComboBox->addItem (tr ("Flat"), smoothing_formula_type::flat);
    _smoothingComboBox->addItem (tr ("Linear"), smoothing_formula_type::linear);
    _smoothingComboBox->addItem (tr ("Smooth"), smoothing_formula_type::smooth);

    connect (_smoothingComboBox, SIGNAL(currentIndexChanged(int)), SLOT(shaping_formula(int)));

    _smoothing_settings_widget->addWidget (_smoothingComboBox);

    _smoothing_radius_slider = new QSlider (Qt::Horizontal, _smoothing_settings_widget);
    _smoothing_radius_slider->setMinimum (smoothing_radius_constants.minimum * smoothing_radius_constants.scale);
    _smoothing_radius_slider->setMaximum (smoothing_radius_constants.maximum * smoothing_radius_constants.scale);
    _smoothing_radius_slider->setMaximumWidth (200);
    connect (_smoothing_radius_slider, SIGNAL (valueChanged (int)), SLOT (smoothing_radius (int)));

    _smoothing_speed_slider = new QSlider (Qt::Horizontal, _smoothing_settings_widget);
    _smoothing_speed_slider->setMinimum (smoothing_speed_constants.minimum * smoothing_speed_constants.scale);
    _smoothing_speed_slider->setMaximum (smoothing_speed_constants.maximum * smoothing_speed_constants.scale);
    _smoothing_speed_slider->setMaximumWidth (200);
    connect (_smoothing_speed_slider, SIGNAL (valueChanged (int)), SLOT (smoothing_speed (int)));

    QLabel* radius_label (new QLabel (tr ("Brush &radius"), _smoothing_settings_widget));
    QLabel* speed_label (new QLabel (tr ("Shaping &speed"), _smoothing_settings_widget));
    _smoothing_radius_percent_label = new QLabel (_smoothing_settings_widget);
    _smoothing_speed_percent_label = new QLabel (_smoothing_settings_widget);

    connect (_smoothing_radius_slider, SIGNAL (valueChanged (int)), _smoothing_radius_percent_label, SLOT (setNum (int)));
    connect (_smoothing_speed_slider,  SIGNAL (valueChanged (int)), _smoothing_speed_percent_label, SLOT (setNum (int)));

    radius_label->setBuddy (_smoothing_radius_slider);
    speed_label->setBuddy (_smoothing_speed_slider);
    _smoothing_radius_percent_label->setBuddy (_smoothing_radius_slider);
    _smoothing_speed_percent_label->setBuddy (_smoothing_speed_slider);

    speed_label->setStyleSheet("margin-left:10px");

    _smoothing_settings_widget->addWidget (radius_label);
    _smoothing_settings_widget->addWidget (_smoothing_radius_slider);
    _smoothing_settings_widget->addWidget (_smoothing_radius_percent_label);
    _smoothing_settings_widget->addWidget (speed_label);
    _smoothing_settings_widget->addWidget (_smoothing_speed_slider);
    _smoothing_settings_widget->addWidget (_smoothing_speed_percent_label);

    //! \note Looks funny, but sets the UI to the default position.
    smoothing_radius (smoothing_radius());
    smoothing_speed (smoothing_speed());
    smoothing_formula (smoothing_formula());
  }

  void MapView::create_paint_settings_widget()
  {
    //! \todo actually do stuff, not only have comments.
    // radius
    // opacity
    // hardness
    // pressure

    // button for swapper

    delete _texturing_settings_widget;

    _texturing_settings_widget = new QToolBar (nullptr);

    _texturingComboBox = new QComboBox (_texturing_settings_widget);

    _texturingComboBox->addItem(QIcon(render_blp_to_pixmap("tileset\\elwynn\\elwynngrassbase.blp",50,50)),"elwynngrassbase","tileset\\elwynn\\elwynngrassbase.blp");

    _texturing_settings_widget->addWidget(_texturingComboBox);

    _texturing_radius_slider = new QSlider (Qt::Horizontal, _texturing_settings_widget);
    _texturing_radius_slider->setMinimum (texturing_radius_constants.minimum * texturing_radius_constants.scale);
    _texturing_radius_slider->setMaximum (texturing_radius_constants.maximum * texturing_radius_constants.scale);
    _texturing_radius_slider->setMaximumWidth(200);
    connect (_texturing_radius_slider, SIGNAL (valueChanged (int)), SLOT (texturing_radius (int)));

    _texturing_opacity_slider = new QSlider (Qt::Horizontal, _texturing_settings_widget);
    _texturing_opacity_slider->setMinimum (texturing_opacity_constants.minimum * texturing_opacity_constants.scale);
    _texturing_opacity_slider->setMaximum (texturing_opacity_constants.maximum * texturing_opacity_constants.scale);
    _texturing_opacity_slider->setMaximumWidth(200);
    connect (_texturing_opacity_slider, SIGNAL (valueChanged (int)), SLOT (texturing_opacity (int)));

    _texturing_hardness_slider = new QSlider (Qt::Horizontal, _texturing_settings_widget);
    _texturing_hardness_slider->setMinimum (texturing_hardness_constants.minimum * texturing_hardness_constants.scale);
    _texturing_hardness_slider->setMaximum (texturing_hardness_constants.maximum * texturing_hardness_constants.scale);
    _texturing_hardness_slider->setMaximumWidth(200);
    connect (_texturing_hardness_slider, SIGNAL (valueChanged (int)), SLOT (texturing_hardness (int)));

    _texturing_pressure_slider = new QSlider (Qt::Horizontal, _texturing_settings_widget);
    _texturing_pressure_slider->setMinimum (texturing_pressure_constants.minimum * texturing_pressure_constants.scale);
    _texturing_pressure_slider->setMaximum (texturing_pressure_constants.maximum * texturing_pressure_constants.scale);
    _texturing_pressure_slider->setMaximumWidth(200);
    connect (_texturing_pressure_slider, SIGNAL (valueChanged (int)), SLOT (texturing_pressure (int)));

    QLabel* radius_label (new QLabel (tr ("Brush &radius"), _texturing_settings_widget));
    QLabel* opacity_label (new QLabel (tr ("Brush &opacity"), _texturing_settings_widget));
    QLabel* hardness_label (new QLabel (tr ("Brush &hardness"), _texturing_settings_widget));
    QLabel* pressure_label (new QLabel (tr ("Brush &pressure"), _texturing_settings_widget));

    _texturing_settings_widget->addWidget (radius_label);
    _texturing_settings_widget->addWidget (_texturing_radius_slider);
    _texturing_settings_widget->addWidget (opacity_label);
    _texturing_settings_widget->addWidget (_texturing_opacity_slider);
    _texturing_settings_widget->addWidget (hardness_label);
    _texturing_settings_widget->addWidget (_texturing_hardness_slider);
    _texturing_settings_widget->addWidget (pressure_label);
    _texturing_settings_widget->addWidget (_texturing_pressure_slider);

    radius_label->setBuddy (_smoothing_radius_slider);
    opacity_label->setBuddy (_texturing_opacity_slider);
    hardness_label->setBuddy (_texturing_hardness_slider);
    pressure_label->setBuddy (_texturing_pressure_slider);

    texturing_radius (texturing_radius());
    texturing_opacity (texturing_opacity());
    texturing_hardness (texturing_hardness());
    texturing_pressure (texturing_pressure());
  }

  void MapView::createToolBar()
  {
      toolBar = new QToolBar(nullptr);

      _toolbar_formula_radio_group = new QButtonGroup;

      QWidget *widget = new QWidget();
      QVBoxLayout *layout = new QVBoxLayout(widget);

      QPushButton *shapingButton = new QPushButton(QIcon(render_blp_to_pixmap("Interface\\ICONS\\INV_Elemental_Mote_Earth01.blp", 40, 40)), "");
      shapingButton->setIconSize (QSize(40, 40));
      shapingButton->setMaximumSize (50, 50);
      shapingButton->setToolTip (tr("Terrain Tool"));
      shapingButton->setStyleSheet ("border:2px solid black; border-radius: 5px; background-color: black; color: white;"); //transparent

      QPushButton *smoothingButton = new QPushButton(QIcon(render_blp_to_pixmap("Interface\\ICONS\\INV_Elemental_Mote_Air01.blp", 40, 40)), "");
      smoothingButton->setIconSize (QSize(40, 40));
      smoothingButton->setMaximumSize (50, 50);
      smoothingButton->setToolTip (tr("Smoothing Tool"));
      smoothingButton->setStyleSheet ("border:2px solid black; border-radius: 5px; background-color: black; color: white;");

      _toolbar_formula_radio_group->addButton (shapingButton, shaping);
      _toolbar_formula_radio_group->addButton (smoothingButton, smoothing);

      layout->addWidget (shapingButton);
      layout->addWidget (smoothingButton);
      toolBar->addWidget (widget);

      connect(_toolbar_formula_radio_group, SIGNAL(buttonClicked(int)), SLOT(set_terrain_editing_mode(int)));

      toolBar->show();
  }







  // --- OBSOLETE GUI ------------------------------------------------------------------------------


#ifdef __OBSOLETE_GUI_ELEMENTS

  //! \todo Make this a member of MapView. Also correctly add the actions below again.
  /*!
    \brief Import a new model form a text file or a hard coded one.
    Imports a model from the import.txt, the wowModelViewer log or just insert some hard coded testing models.
    \param id the id switch the import kind
  */
  void InsertObject( UIFrame* button, int id )
  {
    //! \todo Beautify.

    // Test if there is an selection
    if( !_selection )
      return;
    // the list of the models to import
    std::vector<std::string> m2s_to_add;
    std::vector<std::string> wmos_to_add;

    // the import file
    std::string importFile;

    const char* filesToAdd[15] = {"","","World\\Scale\\humanmalescale.m2","World\\Scale\\50x50.m2","World\\Scale\\100x100.m2","World\\Scale\\250x250.m2","World\\Scale\\500x500.m2","World\\Scale\\1000x1000.m2","World\\Scale\\50yardradiusdisc.m2","World\\Scale\\200yardradiusdisc.m2","World\\Scale\\777yardradiusdisc.m2","World\\Scale\\50yardradiussphere.m2","World\\Scale\\200yardradiussphere.m2","World\\Scale\\777yardradiussphere.m2",""};

    // MODELINSERT FROM TEXTFILE
    // is a source file set in config file?

    switch(id)
      {
         case 0:
         case 14:
         case 15:
         //! \todo do this somehow else or not at all.
          if (false)
          {
            ConfigFile config( "noggIt.conf" );
            config.readInto( importFile, "ImportFile" );
          }
         break;

         case 1:
          importFile="Import.txt"; //  use import.txt in noggit folder!
         break;

         default:
          m2s_to_add.push_back( filesToAdd[id] );
          break;
    }

    std::string lastModel;
    std::string lastWMO;

    if(importFile!="")
    {
      size_t foundString;
      std::string line;
      std::string findThis;
      std::ifstream fileReader(importFile.c_str());
      if (fileReader.is_open())
      {
        while (! fileReader.eof() )
        {
          getline (fileReader,line);
          if(line.find(".m2")!= std::string::npos || line.find(".M2")!= std::string::npos || line.find(".MDX")!= std::string::npos || line.find(".mdx")!= std::string::npos )
          {
            // M2 inside line
            // is it the modelviewer log then cut the log messages out
            findThis =   "Loading model: ";
            foundString = line.find(findThis);
            if(foundString!= std::string::npos)
            {
              // cut path
              line = line.substr( foundString+findThis.size() );
            }

            // swap mdx to m2
            size_t found = line.rfind( ".mdx" );
            if( found != std::string::npos )
              line.replace( found, 4, ".m2" );
            found = line.rfind( ".MDX" );
            if( found != std::string::npos )
              line.replace( found, 4, ".m2" );

            m2s_to_add.push_back( line );
            lastModel = line;
          }
          else if(line.find(".wmo")!= std::string::npos || line.find(".WMO")!= std::string::npos )
          {
            // WMO inside line
            findThis = "Loading WMO ";
            foundString = line.find(findThis);
            // is it the modelviewer log then cut the log messages out
            if(foundString != std::string::npos)
            {
              // cut path
              line = line.substr( foundString+findThis.size() );
            }
            wmos_to_add.push_back(line);
            lastWMO = line;
          }
        }
        fileReader.close();
      }
      else
      {
        // file not exist, no rights ore other error
        LogError << importFile << std::endl;
      }
    }

    const ::math::vector_3d selectionPosition
      (selection::position (*_selection));

    if(id==14)
    {
      LogError << "M2 Problem 14:" << lastModel << " - " << id << std::endl;
      if(lastModel!="")
        if( !MPQFile::exists(lastModel) )
          LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
        else
          _world->addM2(lastModel, selectionPosition );
    }
    else if(id==15)
    {
        LogError << "M2 Problem 15:" << lastModel << " - " << id << std::endl;
      if(lastWMO!="")
        if( !MPQFile::exists(lastWMO) )
          LogError << "Failed adding " << lastWMO << ". It was not in any MPQ." << std::endl;
        else
          _world->addWMO(lastWMO, selectionPosition );
    }
    else
    {

      for( std::vector<std::string>::iterator it = wmos_to_add.begin(); it != wmos_to_add.end(); ++it )
      {

        if( !MPQFile::exists(*it) )
        {
          LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
          continue;
        }

        _world->addWMO(*it, selectionPosition );
      }

      for( std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it )
      {

        if( !MPQFile::exists(*it) )
        {

          LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
          continue;
        }

        _world->addM2(*it, selectionPosition );
      }
    }
    //! \todo Memoryleak: These models will never get deleted.
  }

  void view_texture_palette( UIFrame* /*button*/, int /*id*/ )
  {
    mainGui->TexturePalette->show();
  }

  void change_selected_area_id (int area_id)
  {
    _selected_area_id = area_id;

    if (!areaIDColors.contains (area_id))
    {
      using namespace helper::math;
      areaIDColors[area_id] = ::math::vector_3d ( random::floating_point (0.0f, 1.0f)
                                                , random::floating_point (0.0f, 1.0f)
                                                , random::floating_point (0.0f, 1.0f)
                                                );
    }
  }
#endif

  void MapView::show_texture_switcher()
  {
#ifdef __OBSOLETE_GUI_ELEMENTS
    _texture_switcher->textures_by_selection
      (_world->GetCurrentSelection());
    _texture_switcher->show();
#endif
  }



  void MapView::create_obsolete_gui()
  {
#ifdef __OBSOLETE_GUI_ELEMENTS
    // create main gui object that holds all other gui elements for access ( in the future ;) )
    mainGui = new UIMapViewGUI (_world, this, width(), height());

    mainGui->guiToolbar->current_texture->setClickFunc( view_texture_palette, 0 );

    mainGui->ZoneIDBrowser->setMapID( _world->getMapID() );
    //! \todo Do this differently. Thanks.
    //mainGui->ZoneIDBrowser->setChangeFunc( changeZoneIDValue );

    mainGui->addChild(mainGui->TexturePalette = UITexturingGUI::createTexturePalette(4,8,mainGui));
    mainGui->TexturePalette->hide();
    mainGui->addChild(mainGui->SelectedTexture = UITexturingGUI::createSelectedTexture());
    mainGui->SelectedTexture->hide();
    mainGui->addChild(UITexturingGUI::createTilesetLoader());
    mainGui->addChild(UITexturingGUI::createTextureFilter());
    mainGui->addChild(_map_chunk_properties_window = UITexturingGUI::createMapChunkWindow());
    _map_chunk_properties_window->hide();
#endif
  }

void MapView::updateParent()
{
    noggit::ui::EditorTemplate* editortemplate = (noggit::ui::EditorTemplate*)this->parent();
    editortemplate->addPropBar(_shaping_settings_widget);
    editortemplate->addPropBar(_texturing_settings_widget);
    editortemplate->addPropBar(_smoothing_settings_widget);
    editortemplate->addToolBar(toolBar);
    for (QMenu* menu : _menus)
    {
      editortemplate->addEditorMenu(menu);
    }
}


}
