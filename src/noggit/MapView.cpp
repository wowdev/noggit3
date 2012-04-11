// MapView.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Beket <snipbeket@mail.ru>
// Bernd Lrwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/MapView.h>

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

#include <math/bounded_nearest.h>

#include <helper/qt/signal_blocker.h>

#include <opengl/texture.h>

#include <noggit/Brush.h>
#include <noggit/MapChunk.h>
#include <noggit/WMOInstance.h>
#include <noggit/ModelManager.h>
#include <noggit/World.h>
#include <noggit/Log.h>
#include <noggit/ui/about_widget.h>
#include <noggit/ui/minimap_widget.h>
#include <noggit/ui/help_widget.h>
#include <noggit/ui/cursor_selector.h>
#include <noggit/ui/model_spawner.h>
#include <noggit/MainWindow.h>
#include <noggit/blp_texture.h>

//! \todo Replace all old gui elements with new classes / widgets.
#undef __OBSOLETE_GUI_ELEMENTS

namespace noggit
{
  MapView::MapView ( World* world
                   , qreal viewing_distance
                   , float ah0
                   , float av0
                   , QGLWidget* shared
                   , MainWindow* parent
                   )
    : QGLWidget (parent, shared)
    , _startup_time ()
    , _last_update (0.0)
    , ah( ah0 )
    , av( av0 )
    , _GUIDisplayingEnabled( true )
    , mTimespeed( 0.0f )
    , _world (world)
    , _help_widget (new ui::help_widget (NULL))
    , _about_widget (NULL)
    //  , _about_widget (new ui::about_widget (NULL))
    , _minimap (new ui::minimap_widget (NULL))
    , _model_spawner (new noggit::ui::model_spawner (NULL,shared))
    , _cursor_selector (new ui::cursor_selector (NULL))
    , _is_currently_moving_object (false)
    , _draw_terrain_height_contour (false)
    , _draw_wmo_doodads (true)
    , _draw_fog (true)
    , _draw_lines (false)
    , _draw_doodads (true)
    , _draw_terrain (true)
    , _draw_water (false)
    , _draw_wmos (true)
    , _draw_hole_lines (false)
    , _draw_lighting (true)
    , _holding_left_mouse_button (false)
    , _holding_right_mouse_button (false)
    , _current_terrain_editing_mode (shaping)
    , _terrain_editing_mode_before_2d (_current_terrain_editing_mode)
    , _save_to_minimap_on_next_drawing (false)
    , _shaping_radius (15.0)
    , _shaping_speed (1.0)
    , _shaping_formula (shaping_formula_type::smooth)
    , _shapingComboBox (NULL)
    , _shaping_radius_slider (NULL)
    , _shaping_speed_slider (NULL)
    , _shaping_settings_widget (NULL)
    , _smoothing_radius (15.0)
    , _smoothing_speed (1.0)
    , _smoothing_formula (smoothing_formula_type::smooth)
    , _smoothingComboBox (NULL)
    , _smoothing_radius_slider (NULL)
    , _smoothing_speed_slider (NULL)
    , _smoothing_settings_widget (NULL)
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
    , _clipboard (NULL)
    , _invert_mouse_y_axis (false)

    //! \todo Sort to correct order and rename.
    , moving (0.0f)
    , strafing (0.0f)
    , updown (0.0f)
    , movespd (66.6f)
    , look (false)
    , mViewMode (eViewMode_3D)
    , mainWindow(parent)
  {
    setMinimumSize(500,500);
    setAcceptDrops (true);
    setFocusPolicy (Qt::StrongFocus);
    setMouseTracking (true);

    createGUI();

    //! \todo Dynamic?
    startTimer (40);
    //! \todo QTimerEvent->time_since_startup or something?
    _startup_time.start();
  }

  float mh,mv,rh,rv;

  float keyx,keyy,keyz,keyr,keys;

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
    //delete_object->setShortcuts (QList<QKeySequence>() << QKeySequence::Delete << Qt::Key_Backspace);

    NEW_ACTION (reset_rotation, tr ("Reset object's rotation"), SLOT (reset_selected_object_rotation()), Qt::CTRL + Qt::Key_R);
    NEW_ACTION (snap_object_to_ground, tr ("Snap object to ground"), SLOT (snap_selected_object_to_ground()), Qt::Key_PageDown);


    NEW_TOGGLE_ACTION (current_texture, tr ("Selected texture"), SLOT (toggle_current_texture_visiblity (bool)), 0, false);
    NEW_TOGGLE_ACTION (toggle_minimap, tr ("Show minimap"), SLOT (toggle_minimap (bool)), Qt::Key_M, false);
    NEW_TOGGLE_ACTION (detail_infos, tr ("Object information"), SLOT (toggle_detail_info_window (bool)), Qt::Key_F8, false);


    NEW_TOGGLE_ACTION (doodad_drawing, tr ("Draw doodads"), SLOT (toggle_doodad_drawing (bool)), Qt::Key_F1, _draw_doodads);
    NEW_TOGGLE_ACTION (wmo_doodad_drawing, tr ("Draw doodads inside of WMOs"), SLOT (toggle_wmo_doodad_drawing (bool)), Qt::Key_F2, _draw_wmo_doodads);
    NEW_TOGGLE_ACTION (terrain_drawing, tr ("Draw terrain"), SLOT (toggle_terrain_drawing (bool)), Qt::Key_F3, _draw_terrain);
    NEW_TOGGLE_ACTION (water_drawing, tr ("Draw water"), SLOT (toggle_water_drawing (bool)), Qt::Key_F4, _draw_water);
    NEW_TOGGLE_ACTION (wmo_drawing, tr ("Draw WMOs"), SLOT (toggle_wmo_drawing (bool)), Qt::Key_F6, _draw_wmos);
    NEW_TOGGLE_ACTION (line_drawing, tr ("Draw lines"), SLOT (toggle_line_drawing (bool)), Qt::Key_F7, _draw_lines);
    NEW_TOGGLE_ACTION (hole_line_drawing, tr ("Draw lines for holes"), SLOT (toggle_hole_line_drawing (bool)), Qt::SHIFT + Qt::Key_F7, _draw_hole_lines);
    //! \todo on OSX this shows up as "8" in menu and does not react to the keybinding.
    NEW_TOGGLE_ACTION (contour_drawing, tr ("Draw contours"), SLOT (toggle_contour_drawing (bool)), Qt::Key_F9, _draw_terrain_height_contour);
    NEW_TOGGLE_ACTION (fog_drawing, tr ("Draw fog"), SLOT (toggle_fog_drawing (bool)), Qt::Key_F, _draw_fog);
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


    NEW_ACTION_OTHER (key_bindings, tr ("Key bindings"), _help_widget, SLOT (show()), Qt::Key_H);
    NEW_ACTION_OTHER (about_noggit, tr ("About Noggit"), _about_widget, SLOT (show()), 0);

    NEW_ACTION (save_wdt, tr ("Save WDT"), SLOT (TEST_save_wdt()), 0);
    NEW_ACTION (save_minimap, tr ("Save minimap as raw files"), SLOT (save_minimap()), Qt::Key_P + Qt::SHIFT + Qt::CTRL);
    NEW_ACTION_OTHER (model_spawner, tr ("Add object to map"), _model_spawner, SLOT (show()), Qt::Key_T);

  #undef NEW_ACTION
  #undef NEW_ACTION_OTHER
  #undef NEW_TOGGLE_ACTION

  #ifdef Q_WS_X11
    QMenuBar* menu_bar (new QMenuBar (this));

  #else
    QMenuBar* menu_bar (new QMenuBar (NULL));
  #endif

    QMenu* file_menu (menu_bar->addMenu (tr ("File")));
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

    QMenu* edit_menu (menu_bar->addMenu (tr ("Edit")));
    edit_menu->addAction (copy_object);
    edit_menu->addAction (paste_object);
    edit_menu->addAction (delete_object);
    edit_menu->addSeparator();
    edit_menu->addAction (reset_rotation);
    edit_menu->addAction (snap_object_to_ground);

    QMenu* assist_menu (menu_bar->addMenu (tr ("Assist")));
    QMenu* insertion_menu (assist_menu->addMenu (tr ("Insert helper model")));

    /*
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
    QMenu* view_menu (menu_bar->addMenu (tr ("View")));
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

    QMenu* settings_menu (menu_bar->addMenu (tr ("Settings")));
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

    QMenu* help_menu (menu_bar->addMenu (tr ("Help")));
    help_menu->addAction (key_bindings);
    help_menu->addAction (about_noggit);

    QMenu* debug_menu (menu_bar->addMenu (tr ("Testing and Debugging")));
    debug_menu->addAction (save_wdt);
    debug_menu->addAction (model_spawner);
    debug_menu->addAction (save_minimap);

    QMenu* useless_menu (debug_menu->addMenu (tr ("Stuff that should only be on keys")));
    useless_menu->addAction (turn_around);

    menu_bar->show();
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

  void MapView::dropEvent (QDropEvent* event)
  {
    const QMimeData* mime (event->mimeData());
    if (mime->hasFormat (ui::model_spawner::mime_type()))
    {
      event->accept();
      const QString path (mime->data (ui::model_spawner::mime_type()));

      if (path.endsWith (".m2"))
      {
        _world->addM2 (ModelManager::add (path.toStdString()), _world->_exact_terrain_selection_position);
      }
      else if (path.endsWith (".wmo"))
      {
        _world->addWMO (WMOManager::add (_world, path.toStdString()), _world->_exact_terrain_selection_position);
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

    tick (now, now - _last_update);
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
    _world = NULL;
  }

  void MapView::tick( float /*t*/, float dt )
  {
    dt = std::min( dt, 1.0f );

    ::math::vector_3d dir( 1.0f, 0.0f, 0.0f );
    ::math::vector_3d dirUp( 1.0f, 0.0f, 0.0f );
    ::math::vector_3d dirRight( 0.0f, 0.0f, 1.0f );
    ::math::rotate (0.0f, 0.0f, &dir.x(), &dir.y(), av);
    ::math::rotate (0.0f, 0.0f, &dir.x(), &dir.z(), ah);

    if( _currently_holding_shift )
    {
      dirUp.x (0.0f);
      dirUp.y (1.0f);
      dirRight = ::math::vector_3d (0.0f, 0.0f, 0.0f);
    }
    else if( _currently_holding_control )
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

    nameEntry * Selection = _world->GetCurrentSelection();

    if( Selection )
    {
      qreal keypad_object_move_ratio (0.1);
      // Set move scale and rotate for numpad keys
      if(_currently_holding_control && _currently_holding_shift) keypad_object_move_ratio = 0.05;
      else if(_currently_holding_shift) keypad_object_move_ratio=0.2;
      else if(_currently_holding_control) keypad_object_move_ratio=0.3;

      if( keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0)
      {
        //! \todo On all these, setchanged() is wrong, if models are bigger than a chunk.
        // Move scale and rotate with numpad keys
        if( Selection->type == eEntry_WMO )
        {
          _world->setChanged ( Selection->data.wmo->pos.x()
                             , Selection->data.wmo->pos.z()
                             );

          Selection->data.wmo->pos.x ( Selection->data.wmo->pos.x()
                                     + keyx * keypad_object_move_ratio);
          Selection->data.wmo->pos.y ( Selection->data.wmo->pos.y()
                                     + keyy * keypad_object_move_ratio);
          Selection->data.wmo->pos.z ( Selection->data.wmo->pos.z()
                                     + keyz * keypad_object_move_ratio);
          Selection->data.wmo->dir.y ( Selection->data.wmo->dir.y()
                                     + keyr * keypad_object_move_ratio * 2.0);

          _world->setChanged ( Selection->data.wmo->pos.x()
                             , Selection->data.wmo->pos.z()
                             );
        }

        if( Selection->type == eEntry_Model )
        {

          _world->setChanged ( Selection->data.model->pos.x()
                             , Selection->data.model->pos.z()
                             );

          Selection->data.model->pos.x ( Selection->data.model->pos.x()
                                       + keyx * keypad_object_move_ratio);
          Selection->data.model->pos.y ( Selection->data.model->pos.y()
                                       + keyy * keypad_object_move_ratio);
          Selection->data.model->pos.z ( Selection->data.model->pos.z()
                                       + keyz * keypad_object_move_ratio);
          Selection->data.model->dir.y ( Selection->data.model->dir.y()
                                       + keyr * keypad_object_move_ratio * 2.0);
          Selection->data.model->sc = Selection->data.model->sc
                                    + keys * keypad_object_move_ratio / 50.0;

          _world->setChanged ( Selection->data.model->pos.x()
                             , Selection->data.model->pos.z()
                             );
        }
      }

      ::math::vector_3d ObjPos;
      if( _world->IsSelection( eEntry_Model ) )
      {
        //! \todo  Tell me what this is.
        ObjPos = Selection->data.model->pos - _world->camera;
        ::math::rotate (0.0f, 0.0f, &ObjPos.x(), &ObjPos.y(), av);
        ::math::rotate (0.0f, 0.0f, &ObjPos.x(), &ObjPos.z(), ah);
        ObjPos.x (fabs (ObjPos.x()));
      }

      // moving and scaling objects
      //! \todo  Alternatively automatically align it to the terrain. Also try to move it where the mouse points.
      if( _is_currently_moving_object )
        if( Selection->type == eEntry_WMO )
        {
           _world->setChanged ( Selection->data.wmo->pos.x()
                              , Selection->data.wmo->pos.z()
                              ); // before move
           ObjPos.x (80.0f);
           Selection->data.wmo->pos += mv * dirUp * ObjPos.x();
           Selection->data.wmo->pos -= mh * dirRight * ObjPos.x();
           Selection->data.wmo->extents[0] =
             Selection->data.wmo->pos - ::math::vector_3d (1,1,1);
           Selection->data.wmo->extents[1] =
             Selection->data.wmo->pos + ::math::vector_3d (1,1,1);
           _world->setChanged ( Selection->data.wmo->pos.x()
                              , Selection->data.wmo->pos.z()
                              ); // after move. If moved to another ADT
        }
        else if( Selection->type == eEntry_Model )
          if( _currently_holding_alt )
          {
            _world->setChanged(Selection->data.model->pos.x(),Selection->data.model->pos.z());
            float ScaleAmount;
            ScaleAmount = pow( 2.0f, mv * 4.0f );
            Selection->data.model->sc *= ScaleAmount;
            if(Selection->data.model->sc > 63.9f )
              Selection->data.model->sc = 63.9f;
            else if (Selection->data.model->sc < 0.00098f )
              Selection->data.model->sc = 0.00098f;
          }
          else
          {
            _world->setChanged(Selection->data.model->pos.x(),Selection->data.model->pos.z()); // before move
            ObjPos.x (80.0f);
            Selection->data.model->pos += mv * dirUp * ObjPos.x();
            Selection->data.model->pos -= mh * dirRight * ObjPos.x();
            _world->setChanged(Selection->data.model->pos.x(),Selection->data.model->pos.z()); // after move. If moved to another ADT
          }


      // rotating objects
      if( look )
      {
        float * lTarget = NULL;
        bool lModify = false;

        if( Selection->type == eEntry_Model )
        {
          _world->setChanged ( Selection->data.model->pos.x()
                             , Selection->data.model->pos.z()
                             );
          lModify = _currently_holding_shift | _currently_holding_control | _currently_holding_alt;
          if( _currently_holding_shift )
            lTarget = &Selection->data.model->dir.y();
          else if( _currently_holding_control )
            lTarget = &Selection->data.model->dir.x();
          else if(_currently_holding_alt )
            lTarget = &Selection->data.model->dir.z();
        }
        else if( Selection->type == eEntry_WMO )
        {
          _world->setChanged ( Selection->data.wmo->pos.x()
                             , Selection->data.wmo->pos.z()
                             );
          lModify = _currently_holding_shift | _currently_holding_control | _currently_holding_alt;
          if( _currently_holding_shift )
            lTarget = &Selection->data.wmo->dir.y();
          else if( _currently_holding_control )
            lTarget = &Selection->data.wmo->dir.x();
          else if( _currently_holding_alt )
            lTarget = &Selection->data.wmo->dir.z();
        }

        if( lModify && lTarget )
        {
          *lTarget = *lTarget + rh + rv;

          if( *lTarget > 360.0f )
            *lTarget = *lTarget - 360.0f;
          else if( *lTarget < -360.0f )
            *lTarget = *lTarget + 360.0f;
        }
      }

      mh = 0;
      mv = 0;
      rh = 0;
      rv = 0;

      if( _holding_left_mouse_button && Selection->type==eEntry_MapChunk )
      {
        const ::math::vector_3d& position (_world->_exact_terrain_selection_position);
        float xPos = position.x();
        float yPos = position.y();
        float zPos = position.z();

        switch( _current_terrain_editing_mode )
        {
        case shaping:
          if( mViewMode == eViewMode_3D )
          {
            if( _currently_holding_shift )
            {
              _world->changeTerrain ( xPos
                                    , zPos
                                    , 7.5f * dt * shaping_speed()
                                    , shaping_radius()
                                    , shaping_formula()
                                    );
            }
            else if( _currently_holding_control )
            {
              _world->changeTerrain ( xPos
                                    , zPos
                                    , -7.5f * dt * shaping_speed()
                                    , shaping_radius()
                                    , shaping_formula()
                                    );
            }
          }
          break;

        case smoothing:
          if( mViewMode == eViewMode_3D )
          {
            if( _currently_holding_shift )
            {
              _world->flattenTerrain ( xPos
                                     , zPos
                                     , yPos
                                     , pow( 0.2f, dt ) * smoothing_speed()
                                     , smoothing_radius()
                                     , smoothing_formula()
                                     );
            }
            else if( _currently_holding_control )
            {
              _world->blurTerrain ( xPos
                                  , zPos
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
                                         ? QPointF (xPos, zPos)
                                         : tile_mode_brush_position()
                                         + QPointF ( _world->camera.x()
                                                   , _world->camera.z()
                                                   )
                                         );

            if ( _currently_holding_shift
              && _currently_holding_control
               )
            {
              _world->eraseTextures (brush_position.x(), brush_position.y());
            }
            else if (_currently_holding_control)
            {
#ifdef __OBSOLETE_GUI_ELEMENTS
              mainGui->TexturePicker->getTextures (_world->GetCurrentSelection());
#endif
            }
            else if ( _currently_holding_shift
#ifdef __OBSOLETE_GUI_ELEMENTS
                   && UITexturingGUI::getSelectedTexture()
#endif
                    )
            {
              _world->paintTexture ( brush_position.x()
                                   , brush_position.y()
                                   , brush (_texturing_radius, _texturing_hardness)
                                   , (1.0f - _texturing_opacity) * 255.0f
                                   , 1.0f - pow ( 1.0f - (float)_texturing_pressure
                                                , (float)dt * 10.0f
                                                )
                                   ,
                                   #ifdef __OBSOLETE_GUI_ELEMENTS
                                   UITexturingGUI::getSelectedTexture()
                                   #else
                                   NULL
                                   #endif
                                   );
            }
          }
        break;

        case hole_setting:
          if (_currently_holding_shift)
          {
            // if there is no terain the projection mothod dont work. So get the cords by selection.
            Selection->data.mapchunk->getSelectionCoord( &xPos, &zPos );
            yPos = Selection->data.mapchunk->getSelectionHeight();

            if( mViewMode == eViewMode_3D )
              _world->removeHole( xPos, zPos );
            //else if( mViewMode == eViewMode_2D )
            //  _world->removeHole( CHUNKSIZE * 4.0f * ratio * ( _mouse_position.x() / float( width() ) - 0.5f ) / _tile_mode_zoom + _world->camera.x, CHUNKSIZE * 4.0f * ( _mouse_position.y() / float( height() ) - 0.5f) / _tile_mode_zoom + _world->camera.z );
          }
          else if( _currently_holding_control )
          {
            if( mViewMode == eViewMode_3D )
              _world->addHole( xPos, zPos );
            //else if( mViewMode == eViewMode_2D )
            //  _world->addHole( CHUNKSIZE * 4.0f * ratio * ( _mouse_position.x() / float( width() ) - 0.5f ) / _tile_mode_zoom+_world->camera.x, CHUNKSIZE * 4.0f * ( _mouse_position.y() / float( height() ) - 0.5f) / _tile_mode_zoom+_world->camera.z );
          }
        break;

        case area_id_setting:
          if( _currently_holding_shift  )
          {
            if( mViewMode == eViewMode_3D )
            {
              // draw the selected AreaId on current selected chunk
              nameEntry * lSelection = _world->GetCurrentSelection();
              int mtx,mtz,mcx,mcy;
              mtx = lSelection->data.mapchunk->mt->mPositionX;
              mtz = lSelection->data.mapchunk->mt->mPositionZ ;
              mcx = lSelection->data.mapchunk->px;
              mcy = lSelection->data.mapchunk->py;
              _world->setAreaID( _selected_area_id, mtx,mtz, mcx, mcy );
            }
          }
          else if( _currently_holding_control )
          {
            if( mViewMode == eViewMode_3D )
            {
              // pick areaID from chunk
              _selected_area_id = _world->GetCurrentSelection()->data.mapchunk->areaID;
#ifdef __OBSOLETE_GUI_ELEMENTS
              mainGui->ZoneIDBrowser->setZoneID(_selected_area_id);
#endif
            }
          }

        break;

        case impassable_flag_setting:
          if( _currently_holding_shift  )
          {
            if( mViewMode == eViewMode_3D ) _world->setFlag( true, xPos, zPos );
          }
          else if( _currently_holding_control )
          {
            if( mViewMode == eViewMode_3D ) _world->setFlag( false, xPos, zPos );
          }
        break;
        }
      }
    }

    if( mViewMode != eViewMode_2D )
    {
      if( moving )
        _world->camera += dir * dt * movespd * moving;
      if( strafing )
      {
        ::math::vector_3d right = dir % ::math::vector_3d( 0.0f, 1.0f ,0.0f );
        right.normalize();
        _world->camera += right * dt * movespd * strafing;
      }
      if( updown )
        _world->camera += ::math::vector_3d( 0.0f, dt * movespd * updown, 0.0f );

      _world->lookat = _world->camera + dir;
    }
    else
    {
      if( moving )
        _world->camera.z ( _world->camera.z()
                         - dt * movespd * moving / (_tile_mode_zoom * 1.5f)
                         );
      if( strafing )
        _world->camera.x ( _world->camera.x()
                         + dt * movespd * strafing / (_tile_mode_zoom * 1.5f)
                         );
      if( updown )
        _tile_mode_zoom *= pow( 2.0f, dt * updown * 4.0f );

      _tile_mode_zoom = qBound (0.1, _tile_mode_zoom, 2.0);
    }

    _world->time += mTimespeed * dt;
    _world->animtime += dt * 1000.0f;
    globalTime = static_cast<int>( _world->animtime );

    _world->tick(dt);

#ifdef __OBSOLETE_GUI_ELEMENTS
    if( !_map_chunk_properties_window->hidden() && _world->GetCurrentSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk )
    {
      UITexturingGUI::setChunkWindow( _world->GetCurrentSelection()->data.mapchunk );
    }
#endif

    //! \todo This should only be done when actually needed. (on movement and camera changes as well as modifying an adt)
    _minimap->update();
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

    _world->drawSelection ( _draw_wmo_doodads
                          , _draw_wmos && !selectTerrainOnly
                          , _draw_doodads && !selectTerrainOnly
                          , _draw_terrain
                          );
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
    _world->drawTileMode ( _draw_lines, width() / qreal (height())
                         , _tile_mode_zoom
                         );
    draw_tile_mode_brush();
  }

  void MapView::displayViewMode_3D()
  {
    //! \note Select terrain below mouse, if no item selected or the item is map.
    if ( !_world->IsSelection( eEntry_Model )
      && !_world->IsSelection( eEntry_WMO )
      && _automatically_update_terrain_selection
       )
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

    _world->draw ( _draw_terrain_height_contour
                 , _current_terrain_editing_mode == impassable_flag_setting
                 , _current_terrain_editing_mode == area_id_setting
                 , _current_terrain_editing_mode == hole_setting
                 , brush_radius
                 , brush_radius
                 , _draw_wmo_doodads
                 , _draw_fog
                 , _draw_wmos
                 , _draw_terrain
                 , _draw_doodads
                 , _draw_lines
                 , _draw_hole_lines
                 , _draw_water
                 , _mouse_position
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

    // fog distance or brush radius
    if (event->key() == Qt::Key_Plus)
      if( event->modifiers() & Qt::AltModifier )
      {
        increase_brush_size();
      }
      else if( event->modifiers() & Qt::ShiftModifier && ( !_world->HasSelection() || ( _world->HasSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk) )  )
        _world->fogdistance += 60.0f;// fog change only when no model is selected!
      else
      {
        //change selected model size
        keys=1;
      }

    if (event->key() == Qt::Key_Minus)
      if (event->modifiers() & Qt::AltModifier)
      {
        decrease_brush_size();
      }
      else if( event->modifiers() & Qt::ShiftModifier && ( !_world->HasSelection() || ( _world->HasSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk) )  )
        _world->fogdistance -= 60.0f; // fog change only when no model is selected!
      else
      {
        //change selected model sizesize
        keys=-1;
      }

    // doodads set
    if( event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9 )
    {
      if( _world->IsSelection( eEntry_WMO ) )
      {
        _world->GetCurrentSelection()->data.wmo->doodadset = event->key() - Qt::Key_0;
      }
      else if (event->modifiers() & Qt::ShiftModifier)
      {
        switch (event->key())
        {
          case Qt::Key_1:
            movespd = 15.0f;
            break;

          case Qt::Key_2:
            movespd = 50.0f;
            break;

          case Qt::Key_3:
            movespd = 300.0f;
            break;

          case Qt::Key_4:
            movespd = 1000.0f;
            break;
        }
      }
      else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_6)
      {
        set_terrain_editing_mode
          (terrain_editing_modes (event->key() - Qt::Key_1));
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
    if( _world->IsSelection( eEntry_WMO ) )
      _world->deleteWMOInstance( _world->GetCurrentSelection()->data.wmo->mUniqueID );
    else if( _world->IsSelection( eEntry_Model ) )
      _world->deleteModelInstance( _world->GetCurrentSelection()->data.model->d1 );
  }

  void MapView::paste_object()
  {
    if( _world->HasSelection() && _clipboard )
    {
      nameEntry lClipboard (*_clipboard);
      ::math::vector_3d position;
      switch( _world->GetCurrentSelection()->type )
       {
        case eEntry_Model:
          position = _world->GetCurrentSelection()->data.model->pos;
          break;
        case eEntry_WMO:
          position = _world->GetCurrentSelection()->data.wmo->pos;
          break;
        case eEntry_MapChunk:
          position = _world->GetCurrentSelection()->data.mapchunk->
                             GetSelectionPosition();
          break;
        default:
          break;
      }
      _world->addModel ( lClipboard
                       , position
                       , _copy_size_randomization
                       , _copy_position_randomization
                       , _copy_rotation_randomization
                       );
    }
  }

  void MapView::copy_selected_object()
  {
    if( _world->HasSelection() )
    {
      _clipboard = new nameEntry (*_world->GetCurrentSelection());
    }
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
    if( _world->IsSelection( eEntry_WMO ) )
    {
      _world->GetCurrentSelection()->data.wmo->resetDirection();
      _world->setChanged(_world->GetCurrentSelection()->data.wmo->pos.x(), _world->GetCurrentSelection()->data.wmo->pos.z());
    }
    else if( _world->IsSelection( eEntry_Model ) )
    {
      _world->GetCurrentSelection()->data.model->resetDirection();
      _world->setChanged(_world->GetCurrentSelection()->data.model->pos.x(), _world->GetCurrentSelection()->data.model->pos.z());
    }
  }

  void MapView::snap_selected_object_to_ground()
  {
    if( _world->IsSelection( eEntry_WMO ) )
    {
      ::math::vector_3d t ( _world->GetCurrentSelection()->data.wmo->pos.x(), _world->GetCurrentSelection()->data.wmo->pos.z(), 0 );
      _world->GetVertex( _world->GetCurrentSelection()->data.wmo->pos.x(), _world->GetCurrentSelection()->data.wmo->pos.z(), &t );
      _world->GetCurrentSelection()->data.wmo->pos = t;
      _world->setChanged(_world->GetCurrentSelection()->data.wmo->pos.x(), _world->GetCurrentSelection()->data.wmo->pos.z());

    }
    else if( _world->IsSelection( eEntry_Model ) )
    {
      ::math::vector_3d t ( _world->GetCurrentSelection()->data.model->pos.x(), _world->GetCurrentSelection()->data.model->pos.z(), 0 );
      _world->GetVertex( _world->GetCurrentSelection()->data.model->pos.x(), _world->GetCurrentSelection()->data.model->pos.z(), &t );
      _world->GetCurrentSelection()->data.model->pos = t;
      _world->setChanged(_world->GetCurrentSelection()->data.model->pos.x(), _world->GetCurrentSelection()->data.model->pos.z());
    }
  }

  void MapView::toggle_interface()
  {
    _GUIDisplayingEnabled = !_GUIDisplayingEnabled;
  }

  static const qreal time_speed_step_length (90.0);
  void MapView::increase_time_speed()
  {
    mTimespeed += time_speed_step_length;
  }
  void MapView::decrease_time_speed()
  {
    mTimespeed -= time_speed_step_length;
  }

  void MapView::toggle_terrain_texturing_mode()
  {
    //! \todo This is BAD global state! Use a stack<settings_type> as member.
    static bool in_texturing_mode (false);
    static bool backup_draw_doodads;
    static bool backup_draw_wmo_doodads;
    static bool backup_draw_terrain_height_contour;
    static bool backup_draw_wmos;
    static bool backup_draw_detailselect;
    static bool backup_draw_fog;
    static bool backup_draw_terrain;

    if( !in_texturing_mode )
    {
      backup_draw_doodads = _draw_doodads;
      backup_draw_wmo_doodads = _draw_wmo_doodads;
      backup_draw_terrain_height_contour = _draw_terrain_height_contour;
      backup_draw_wmos = _draw_wmos;
      backup_draw_fog = _draw_fog;
      backup_draw_terrain = _draw_terrain;

      _draw_doodads = false;
      _draw_wmo_doodads = false;
      _draw_terrain_height_contour = true;
      _draw_wmos = false;
      _draw_terrain = true;
      _draw_fog = false;
    }
    else
    {
      _draw_doodads = backup_draw_doodads;
      _draw_wmo_doodads = backup_draw_wmo_doodads;
      _draw_terrain_height_contour = backup_draw_terrain_height_contour;
      _draw_wmos = backup_draw_wmos;
      _draw_terrain = backup_draw_terrain;
      _draw_fog = backup_draw_fog;
    }
    in_texturing_mode = !in_texturing_mode;
  }

  void MapView::toggle_auto_selecting (bool value)
  {
    _automatically_update_terrain_selection = value;
  }

  /// --- Drawing toggles --------------------------------------------------------

  void MapView::toggle_doodad_drawing (bool value)
  {
    _draw_doodads = value;
  }
  void MapView::toggle_water_drawing (bool value)
  {
    _draw_water = value;
  }
  void MapView::toggle_terrain_drawing (bool value)
  {
    _draw_terrain = value;
  }
  void MapView::toggle_wmo_doodad_drawing (bool value)
  {
    _draw_wmo_doodads = value;
  }
  void MapView::toggle_line_drawing (bool value)
  {
    _draw_lines = value;
  }
  void MapView::toggle_wmo_drawing (bool value)
  {
    _draw_wmos = value;
  }
  void MapView::toggle_hole_line_drawing (bool value)
  {
    _draw_hole_lines = value;
  }
  void MapView::toggle_lighting (bool value)
  {
    _draw_lighting = value;
  }


  //! \todo these should be symetrical, so maybe combine.
  void MapView::increase_brush_size()
  {
    switch( _current_terrain_editing_mode )
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

  void MapView::toggle_contour_drawing (bool value)
  {
    _draw_terrain_height_contour = value;
  }

  void MapView::toggle_fog_drawing (bool value)
  {
    _draw_fog = value;
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
    if (event->button() == Qt::MidButton)
    {
      if (_world->HasSelection())
      {
        _is_currently_moving_object = true;
      }
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
      look = true;
    }
  }

  void MapView::mouseReleaseEvent (QMouseEvent* event)
  {
    if (event->button() == Qt::LeftButton)
    {
      _holding_left_mouse_button = false;

      if (!key_w && moving > 0.0f)
      {
        moving = 0.0f;
      }

      if (mViewMode == eViewMode_2D)
      {
        strafing = 0;
        moving = 0;
      }
    }
    if (event->button() == Qt::RightButton)
    {
      _holding_right_mouse_button = false;

      look = false;

      if (!key_w && moving > 0.0f)
      {
        moving = 0.0f;
      }

      if (mViewMode == eViewMode_2D)
      {
        updown = 0;
      }
    }
    if (event->button() == Qt::MidButton)
    {
      _is_currently_moving_object = false;
    }
  }

  void MapView::mouseMoveEvent (QMouseEvent* event)
  {
    static const float XSENS (15.0f);
    static const float YSENS (15.0f);

    const QPoint relative_move (event->pos() - _mouse_position);

    if (look && event->modifiers() == Qt::NoModifier)
    {
      ah += relative_move.x() / XSENS;
      av += (_invert_mouse_y_axis ? 1.0 : -1.0) * relative_move.y() / YSENS;

      av = qBound (-80.0f, av, 80.0f);
    }

    if( _is_currently_moving_object )
    {
      const qreal ratio (height() / qreal (width()));
      mh = -ratio * relative_move.x() / qreal (width());
      mv = -relative_move.y() / qreal (height());
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
        break;

      case smoothing:
        shaping_radius (shaping_radius() - relative_move.x() / XSENS);
        break;

      case texturing:
        texturing_radius (texturing_radius() - relative_move.x() / XSENS);
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

    _draw_hole_lines = mode == hole_setting;

    _shaping_settings_widget->hide();
    _smoothing_settings_widget->hide();
//    _texturing_settings_widget->hide();

    switch (mode)
    {
    case shaping:
      mainWindow->setToolBar(_shaping_settings_widget);
      break;

    case smoothing:
      mainWindow->setToolBar(_smoothing_settings_widget);
      break;

//    case texturing:
//      _texturing_settings_widget->show();
//      break;

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
    _world->moveHeight ( tile_below_camera (_world->camera.x())
                       , tile_below_camera (_world->camera.z())
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
    _world->setBaseTexture ( tile_below_camera (_world->camera.x())
                           , tile_below_camera (_world->camera.z())
                           , NULL
                           );
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

    _shapingComboBox->addItem(tr ("Flat"), shaping_formula_type::flat);
    _shapingComboBox->addItem(tr ("Linear"), shaping_formula_type::linear);
    _shapingComboBox->addItem(tr ("Smooth"), shaping_formula_type::smooth);
    _shapingComboBox->addItem(tr ("Polynomial"), shaping_formula_type::polynomial);
    _shapingComboBox->addItem(tr ("Trigonometric"), shaping_formula_type::trigonometric);
    _shapingComboBox->addItem(tr ("Square"), shaping_formula_type::square);

    connect(_shapingComboBox,SIGNAL(currentIndexChanged(int)),SLOT(shaping_formula(int)));

    _shaping_settings_widget->addWidget(_shapingComboBox);

    _shaping_radius_slider = new QSlider (Qt::Horizontal, _shaping_settings_widget);
    _shaping_radius_slider->setMinimum (shaping_radius_constants.minimum * shaping_radius_constants.scale);
    _shaping_radius_slider->setMaximum (shaping_radius_constants.maximum * shaping_radius_constants.scale);
    _shaping_radius_slider->setMaximumWidth(200);
    connect (_shaping_radius_slider, SIGNAL (valueChanged (int)), SLOT (shaping_radius (int)));

    _shaping_speed_slider = new QSlider (Qt::Horizontal, _shaping_settings_widget);
    _shaping_speed_slider->setMinimum (shaping_speed_constants.minimum * shaping_speed_constants.scale);
    _shaping_speed_slider->setMaximum (shaping_speed_constants.maximum * shaping_speed_constants.scale);
    _shaping_speed_slider->setMaximumWidth(200);
    connect (_shaping_speed_slider, SIGNAL (valueChanged (int)), SLOT (shaping_speed (int)));

    QLabel* radius_label (new QLabel (tr ("Brush &radius"), _shaping_settings_widget));
    QLabel* speed_label (new QLabel (tr ("Shaping &speed"), _shaping_settings_widget));

    radius_label->setBuddy (_shaping_radius_slider);
    speed_label->setBuddy (_shaping_speed_slider);

    _shaping_settings_widget->addWidget(radius_label);
    _shaping_settings_widget->addWidget (_shaping_radius_slider);
    _shaping_settings_widget->addWidget(speed_label);
    _shaping_settings_widget->addWidget (_shaping_speed_slider);

    //! \note Looks funny, but sets the UI to the default position.
    shaping_radius (shaping_radius());
    shaping_speed (shaping_speed());
    shaping_formula (shaping_formula());
  }

  void MapView::create_smoothing_settings_widget()
  {
    delete _smoothing_settings_widget;

    _smoothing_settings_widget = new QToolBar (NULL);

    _smoothingComboBox = new QComboBox (this);

    _smoothingComboBox->addItem(tr ("Flat"), smoothing_formula_type::flat);
    _smoothingComboBox->addItem(tr ("Linear"), smoothing_formula_type::linear);
    _smoothingComboBox->addItem(tr ("Smooth"), smoothing_formula_type::smooth);

    connect(_smoothingComboBox,SIGNAL(currentIndexChanged(int)),SLOT(shaping_formula(int)));

    _smoothing_settings_widget->addWidget(_smoothingComboBox);

    _smoothing_radius_slider = new QSlider (Qt::Horizontal, _smoothing_settings_widget);
    _smoothing_radius_slider->setMinimum (smoothing_radius_constants.minimum * smoothing_radius_constants.scale);
    _smoothing_radius_slider->setMaximum (smoothing_radius_constants.maximum * smoothing_radius_constants.scale);
    _smoothing_radius_slider->setMaximumWidth(200);
    connect (_smoothing_radius_slider, SIGNAL (valueChanged (int)), SLOT (smoothing_radius (int)));

    _smoothing_speed_slider = new QSlider (Qt::Horizontal, _smoothing_settings_widget);
    _smoothing_speed_slider->setMinimum (smoothing_speed_constants.minimum * smoothing_speed_constants.scale);
    _smoothing_speed_slider->setMaximum (smoothing_speed_constants.maximum * smoothing_speed_constants.scale);
    _smoothing_speed_slider->setMaximumWidth(200);
    connect (_smoothing_speed_slider, SIGNAL (valueChanged (int)), SLOT (smoothing_speed (int)));

    QLabel* radius_label (new QLabel (tr ("Brush &radius"), _smoothing_settings_widget));
    QLabel* speed_label (new QLabel (tr ("Shaping &speed"), _smoothing_settings_widget));

    radius_label->setBuddy (_smoothing_radius_slider);
    speed_label->setBuddy (_smoothing_speed_slider);

    _smoothing_settings_widget->addWidget (radius_label);
    _smoothing_settings_widget->addWidget (_smoothing_radius_slider);
    _smoothing_settings_widget->addWidget (speed_label);
    _smoothing_settings_widget->addWidget (_smoothing_speed_slider);

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
  }

  void MapView::createToolBar()
  {
      QToolBar *toolBar = new QToolBar(NULL);

      _toolbar_formula_radio_group = new QButtonGroup;

      QWidget *widget = new QWidget();
      QVBoxLayout *layout = new QVBoxLayout(widget);

      QPushButton *shapingButton = new QPushButton(QIcon(render_blp_to_pixmap("Interface\\ICONS\\INV_Elemental_Mote_Earth01.blp",50,50)), "");
      shapingButton->setIconSize(QSize(50,50));
      QPushButton *smoothingButton = new QPushButton(QIcon(render_blp_to_pixmap("Interface\\ICONS\\INV_Elemental_Mote_Air01.blp",50,50)), "");
      smoothingButton->setIconSize(QSize(50,50));

      _toolbar_formula_radio_group->addButton(shapingButton, shaping);
      _toolbar_formula_radio_group->addButton(smoothingButton, smoothing);

      layout->addWidget(shapingButton);
      layout->addWidget(smoothingButton);
      toolBar->addWidget(widget);

      connect(_toolbar_formula_radio_group,SIGNAL(buttonClicked(int)),SLOT(set_terrain_editing_mode(int)));

      mainWindow->setToolBar(toolBar,Qt::RightToolBarArea);
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
    if( !_world->HasSelection() )
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


    ::math::vector_3d selectionPosition;
    switch( _world->GetCurrentSelection()->type )
    {
      case eEntry_Model:
        selectionPosition = _world->GetCurrentSelection()->data.model->pos;
        break;
      case eEntry_WMO:
        selectionPosition = _world->GetCurrentSelection()->data.wmo->pos;
        break;
      case eEntry_MapChunk:
        selectionPosition = _world->GetCurrentSelection()->data.mapchunk->GetSelectionPosition();
        break;
    }


    if(id==14)
    {
      LogError << "M2 Problem 14:" << lastModel << " - " << id << std::endl;
      if(lastModel!="")
        if( !MPQFile::exists(lastModel) )
          LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
        else
          _world->addM2( ModelManager::add( lastModel ), selectionPosition );
    }
    else if(id==15)
    {
        LogError << "M2 Problem 15:" << lastModel << " - " << id << std::endl;
      if(lastWMO!="")
        if( !MPQFile::exists(lastWMO) )
          LogError << "Failed adding " << lastWMO << ". It was not in any MPQ." << std::endl;
        else
          _world->addWMO( WMOManager::add( lastWMO ), selectionPosition );
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

        _world->addWMO( WMOManager::add( *it ), selectionPosition );
      }

      for( std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it )
      {

        if( !MPQFile::exists(*it) )
        {

          LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
          continue;
        }

        _world->addM2( ModelManager::add( *it ), selectionPosition );
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
    _texture_switcher->textures_by_selection (_world->GetCurrentSelection());
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





}
