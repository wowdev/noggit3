// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Brush.h> // brush
#include <noggit/ConfigFile.h>
#include <noggit/DBC.h>
#include <noggit/Environment.h>
#include <noggit/Environment.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Project.h>
#include <noggit/Settings.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/application.h> // app.getStates(), gPop, app.getArial14(), arial...
#include <noggit/map_index.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/CursorSwitcher.h> // cursor_switcher
#include <noggit/ui/DetailInfos.h> // detailInfos
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/Help.h>
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/TexturePicker.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/Toolbar.h> // noggit::ui::toolbar
#include <noggit/ui/Water.h>
#include <noggit/ui/WaterSaveWarning.h>
#include <noggit/ui/ZoneIDBrowser.h>
#include <noggit/ui/main_window.hpp>
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/shader_tool.hpp>
#include <noggit/ui/terrain_tool.hpp>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/texturing_tool.hpp>
#include <opengl/matrix.hpp>
#include <opengl/scoped.hpp>

#include "revision.h"

#include <boost/filesystem.hpp>

#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

#ifdef _WIN32
#include <external/wacom/MSGPACK.H>
#define PACKETDATA  (PK_BUTTONS | PK_NORMAL_PRESSURE)
#define PACKETMODE  PK_BUTTONS
#include <external/wacom/PKTDEF.H>
#include <external/wacom/Utils.h>
#include <direct.h>
#include <windows.h>
#include <winerror.h>
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
HINSTANCE hInst;
char* gpszProgramName = "Noggit3";
static LOGCONTEXT  glogContext = { 0 };
#endif

static const float XSENS = 15.0f;
static const float YSENS = 15.0f;

void MapView::set_editing_mode (editing_mode mode)
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  _terrain->hide();
  _flatten_blur->hide();
  _texturing->hide();
  _vertex_shading->hide();
  _water->hide();
  TexturePicker->hide();
  _object->hide();
  objectEditor->modelImport->hide();
  objectEditor->rotationEditor->hide();
  _areaid->hide();
  TexturePalette->hide();

  switch (mode)
  {
  case editing_mode::ground:
    _terrain->show();
    break;
  case editing_mode::flatten_blur:
    _flatten_blur->show();
    break;
  case editing_mode::paint:
    _texturing->show();
    break;
  case editing_mode::areaid:
    _areaid->show();
    break;
  case editing_mode::water:
    _water->show();
    break;
  case editing_mode::mccv:
    _vertex_shading->show();
    break;
  case editing_mode::object:
    _object->show();
  }

  terrainMode = mode;
  _toolbar->check_tool (mode);
}

void MapView::ResetSelectedObjectRotation()
{
  if (_world->IsSelection(eEntry_WMO))
  {
    WMOInstance* wmo = boost::get<selected_wmo_type> (*_world->GetCurrentSelection());
    _world->updateTilesWMO(wmo);
    wmo->resetDirection();
    _world->updateTilesWMO(wmo);
  }
  else if (_world->IsSelection(eEntry_Model))
  {
    ModelInstance* m2 = boost::get<selected_model_type> (*_world->GetCurrentSelection());
    _world->updateTilesModel(m2);
    m2->resetDirection();
    m2->recalcExtents();
    _world->updateTilesModel(m2);
  }
}

void MapView::SnapSelectedObjectToGround()
{
  if (_world->IsSelection(eEntry_WMO))
  {
    WMOInstance* wmo = boost::get<selected_wmo_type> (*_world->GetCurrentSelection());
    math::vector_3d t = math::vector_3d(wmo->pos.x, wmo->pos.z, 0);
    _world->GetVertex(wmo->pos.x, wmo->pos.z, &t);
    wmo->pos.y = t.y;
    _world->updateTilesWMO(wmo);
  }
  else if (_world->IsSelection(eEntry_Model))
  {
    ModelInstance* m2 = boost::get<selected_model_type> (*_world->GetCurrentSelection());
    math::vector_3d t = math::vector_3d(m2->pos.x, m2->pos.z, 0);
    _world->GetVertex(m2->pos.x, m2->pos.z, &t);
    m2->pos.y = t.y;
    _world->updateTilesModel(m2);
  }
}


void MapView::DeleteSelectedObject()
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  if (_world->IsSelection(eEntry_WMO))
  {
    _world->deleteWMOInstance(boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->mUniqueID);
  }
  else if (_world->IsSelection(eEntry_Model))
  {
    _world->deleteModelInstance(boost::get<selected_model_type> (*_world->GetCurrentSelection())->d1);
  }
}

void MapView::insert_last_m2_from_wmv()
{
  // Test if there is an selection
  if (!_world->HasSelection())
  {
    return;
  }

  std::string wmv_log_file (Settings::getInstance()->wmvLogFile);
  std::string lastModel;
  std::string line;
  std::ifstream fileReader(wmv_log_file.c_str());

  if (fileReader.is_open())
  {
    while (!fileReader.eof())
    {
      getline(fileReader, line);
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);
      std::regex regex("([a-z]+\\\\([a-z0-9_ ]+\\\\)*[a-z0-9_ ]+\\.(mdx|m2))");
      std::smatch match;

      if (std::regex_search (line, match, regex))
      {
        lastModel = match.str(0);
        size_t found = lastModel.rfind(".mdx");
        if (found != std::string::npos)
        {
          lastModel.replace(found, 4, ".m2");
        }
      }
    }
  }
  else
  {
    // file not exist, no rights or other error
    LogError << wmv_log_file << std::endl;
  }

  math::vector_3d selectionPosition;
  switch (_world->GetCurrentSelection()->which())
  {
  case eEntry_Model:
    selectionPosition = boost::get<selected_model_type> (*_world->GetCurrentSelection())->pos;
    break;
  case eEntry_WMO:
    selectionPosition = boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->pos;
    break;
  case eEntry_MapChunk:
    selectionPosition = boost::get<selected_chunk_type> (*_world->GetCurrentSelection()).position;
    break;
  }

  if (lastModel != "")
  {
    if (!MPQFile::exists(lastModel))
    {
      LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
    }
    else
    {
      _world->addM2(lastModel, selectionPosition, false);
    }
  }
}

void MapView::insert_last_wmo_from_wmv()
{
  if (!_world->HasSelection())
  {
    return;
  }

  std::string wmv_log_file (Settings::getInstance()->wmvLogFile);
  std::string lastWMO;
  std::string line;
  std::ifstream fileReader(wmv_log_file.c_str());

  if (fileReader.is_open())
  {
    while (!fileReader.eof())
    {
      getline(fileReader, line);
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);
      std::regex regex("([a-z]+\\\\([a-z0-9_ ]+\\\\)*[a-z0-9_ ]+\\.(wmo))");
      std::smatch match;

      if (std::regex_search (line, match, regex))
      {
        lastWMO = match.str(0);
      }
    }
  }
  else
  {
    // file not exist, no rights or other error
    LogError << wmv_log_file << std::endl;
  }


  math::vector_3d selectionPosition;
  switch (_world->GetCurrentSelection()->which())
  {
  case eEntry_Model:
    selectionPosition = boost::get<selected_model_type> (*_world->GetCurrentSelection())->pos;
    break;
  case eEntry_WMO:
    selectionPosition = boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->pos;
    break;
  case eEntry_MapChunk:
    selectionPosition = boost::get<selected_chunk_type> (*_world->GetCurrentSelection()).position;
    break;
  }

  if (lastWMO != "")
  {
    if (!MPQFile::exists(lastWMO))
    {
      LogError << "Failed adding " << lastWMO << ". It was not in any MPQ." << std::endl;
    }
    else
    {
      _world->addWMO(lastWMO, selectionPosition, false);
    }
  }
  //! \todo Memoryleak: These models will never get deleted.
}



void MapView::changeZoneIDValue (int set)
{
  _selected_area_id = set;
}


void MapView::createGUI()
{
  _terrain = new QDockWidget ("Raise / Lower", this);
  _terrain->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  _terrain->setWidget (terrainTool = new noggit::ui::terrain_tool());
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _terrain);
  connect (this, &QObject::destroyed, _terrain, &QObject::deleteLater);

  _flatten_blur = new QDockWidget ("Flatten / Blur", this);
  _flatten_blur->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  _flatten_blur->setWidget (flattenTool = new noggit::ui::flatten_blur_tool());
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _flatten_blur);
  connect (this, &QObject::destroyed, _flatten_blur, &QObject::deleteLater);

  _texturing = new QDockWidget ("Texturing", this);
  _texturing->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  _texturing->setWidget (texturingTool = new noggit::ui::texturing_tool (&_camera.position));
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _texturing);
  connect (this, &QObject::destroyed, _texturing, &QObject::deleteLater);

  _areaid = new QDockWidget ("Area ID", this);
  _areaid->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  _areaid->setWidget (ZoneIDBrowser = new noggit::ui::zone_id_browser());
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _areaid);
  connect (this, &QObject::destroyed, _areaid, &QObject::deleteLater);

  _water = new QDockWidget ("Raise / Lower", this);
  _water->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  _water->setWidget (guiWater = new noggit::ui::water());
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _water);
  connect (this, &QObject::destroyed, _water, &QObject::deleteLater);

  _vertex_shading = new QDockWidget ("Vertex Shading", this);
  _vertex_shading->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  _vertex_shading->setWidget (shaderTool = new noggit::ui::shader_tool (cursor_color));
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _vertex_shading);
  connect (this, &QObject::destroyed, _vertex_shading, &QObject::deleteLater);

  _object = new QDockWidget ("Object", this);
  _object->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  _object->setWidget (objectEditor = new noggit::ui::object_editor(this));
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _object);
  connect (this, &QObject::destroyed, _object, &QObject::deleteLater);


  TexturePalette = new noggit::ui::tileset_chooser;
  connect ( texturingTool->current_texture, &noggit::ui::current_texture::clicked
          , [=]
            {
              TexturePalette->setVisible (!TexturePalette->isVisible());
            }
          );
  connect ( TexturePalette, &noggit::ui::tileset_chooser::selected
          , [=] (std::string const& filename)
            {
              makeCurrent();
              opengl::context::scoped_setter const _ (::gl, context());

              noggit::ui::selected_texture::set (filename);
              texturingTool->current_texture->set_texture (filename);
            }
          );


  // DetailInfoWindow
  guidetailInfos = new noggit::ui::detail_infos;
  guidetailInfos->hide();

  TexturePicker = new noggit::ui::texture_picker (texturingTool->current_texture);
  TexturePicker->hide();

  connect ( TexturePicker, &noggit::ui::texture_picker::shift_left
          , [=]
            {
              makeCurrent();
              opengl::context::scoped_setter const _ (::gl, context());
              TexturePicker->shiftSelectedTextureLeft();
            }
          );
  connect ( TexturePicker, &noggit::ui::texture_picker::shift_right
          , [=]
            {
              makeCurrent();
              opengl::context::scoped_setter const _ (::gl, context());
              TexturePicker->shiftSelectedTextureRight();
            }
          );

  ZoneIDBrowser->setMapID(_world->getMapID());
  ZoneIDBrowser->setChangeFunc([this] (int id){ changeZoneIDValue (id); });
  tool_settings_x = width() - 186;
  tool_settings_y = 38;

  terrainTool->storeCursorPos (&_cursor_pos);

  _toolbar = new noggit::ui::toolbar([this] (editing_mode mode) { set_editing_mode (mode); });
  _main_window->addToolBar(Qt::LeftToolBarArea, _toolbar);
  connect (this, &QObject::destroyed, _toolbar, &QObject::deleteLater);

  auto file_menu (_main_window->menuBar()->addMenu ("Editor"));
  connect (this, &QObject::destroyed, file_menu, &QObject::deleteLater);

  auto edit_menu (_main_window->menuBar()->addMenu ("Edit"));
  connect (this, &QObject::destroyed, edit_menu, &QObject::deleteLater);

  auto assist_menu (_main_window->menuBar()->addMenu ("Assist"));
  connect (this, &QObject::destroyed, assist_menu, &QObject::deleteLater);

  auto view_menu (_main_window->menuBar()->addMenu ("View"));
  connect (this, &QObject::destroyed, view_menu, &QObject::deleteLater);

  auto help_menu (_main_window->menuBar()->addMenu ("Help"));
  connect (this, &QObject::destroyed, help_menu, &QObject::deleteLater);

#define ADD_ACTION(menu, name, shortcut, on_action)               \
  {                                                               \
    auto action (menu->addAction (name));                         \
    action->setShortcut (QKeySequence (shortcut));                \
    connect (action, &QAction::triggered, on_action);             \
  }

#define ADD_ACTION_NS(menu, name, on_action)                      \
  {                                                               \
    auto action (menu->addAction (name));                         \
    connect (action, &QAction::triggered, on_action);             \
  }

#define ADD_TOGGLE(menu_, name_, shortcut_, property_)            \
  do                                                              \
  {                                                               \
    QAction* action (new QAction (name_, this));                  \
    action->setShortcut (QKeySequence (shortcut_));               \
    action->setCheckable (true);                                  \
    action->setChecked (property_.get());                         \
    menu_->addAction (action);                                    \
    connect ( action, &QAction::toggled                           \
            , &property_, &noggit::bool_toggle_property::set      \
            );                                                    \
    connect ( &property_, &noggit::bool_toggle_property::changed  \
            , action, &QAction::setChecked                        \
            );                                                    \
  }                                                               \
  while (false)

#define ADD_TOGGLE_NS(menu_, name_, property_)                    \
  do                                                              \
  {                                                               \
    QAction* action (new QAction (name_, this));                  \
    action->setCheckable (true);                                  \
    action->setChecked (property_.get());                         \
    menu_->addAction (action);                                    \
    connect ( action, &QAction::toggled                           \
            , &property_, &noggit::bool_toggle_property::set      \
            );                                                    \
    connect ( &property_, &noggit::bool_toggle_property::changed  \
            , action, &QAction::setChecked                        \
            );                                                    \
  }                                                               \
  while (false)



  ADD_ACTION (file_menu, "save current tile", "Ctrl+Shift+S", [this] { prompt_save_current(); });
  ADD_ACTION ( file_menu
             , "save changed tiles"
             , QKeySequence::Save
             , [this]
               {
                 makeCurrent();
                 opengl::context::scoped_setter const _ (::gl, context());
                 _world->mapIndex.saveChanged();
               }
             );
  ADD_ACTION ( file_menu
             , "save all tiles"
             , "Ctrl+Shift+A"
             , [this]
               {
                 makeCurrent();
                 opengl::context::scoped_setter const _ (::gl, context());
                 _world->mapIndex.saveall();
               }
             );
  ADD_ACTION ( file_menu
             , "reload tile"
             , "Shift+J"
             , [this]
               {
                 makeCurrent();
                 opengl::context::scoped_setter const _ (::gl, context());
                 _world->mapIndex.reloadTile (_camera.position);
               }
             );

  file_menu->addSeparator();
  ADD_ACTION (file_menu, "exit", QKeySequence::Quit, [this] { _main_window->prompt_exit(); });


  //! \todo sections are not rendered on all platforms. one should
  //! probably do separator+disabled entry to force rendering
  edit_menu->addSection ("selected object");
  ADD_ACTION (edit_menu, "delete", Qt::Key_Delete, [this] { DeleteSelectedObject(); });
  ADD_ACTION (edit_menu, "reset rotation", "Ctrl+R", [this] { ResetSelectedObjectRotation(); });
  ADD_ACTION (edit_menu, "set to ground", Qt::Key_PageDown, [this] { SnapSelectedObjectToGround(); });

  edit_menu->addSection ("options");
  ADD_TOGGLE_NS (edit_menu, "auto select mode (broken)", _auto_selecting_mode);


  assist_menu->addSection ("Model");
  ADD_ACTION (assist_menu, "Last m2 from WMV", "Shift+V", [this] { insert_last_m2_from_wmv(); });
  ADD_ACTION (assist_menu, "Last WMO from WMV", "Alt+V", [this] { insert_last_wmo_from_wmv(); });
  ADD_ACTION_NS (assist_menu, "Helper models", [this] { HelperModels->show(); });

  assist_menu->addSection ("Current ADT");
  ADD_ACTION_NS ( assist_menu
                , "Set Area ID"
                , [this]
                  {
                    if (_selected_area_id != -1)
                    {
                      _world->setAreaID(_camera.position, _selected_area_id, true);
                    }
                  }
                );
  ADD_ACTION_NS ( assist_menu
                , "Clear height map"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->clearHeight(_camera.position);
                  }
                );

  ADD_ACTION_NS ( assist_menu
                , "Clear texture"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->setBaseTexture(_camera.position);
                  }
                );
  ADD_ACTION_NS ( assist_menu
                , "Clear models"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->clearAllModelsOnADT(_camera.position);
                  }
                );
  ADD_ACTION_NS ( assist_menu
                , "Clear duplicate models"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->delete_duplicate_model_and_wmo_instances();
                  }
                );

  assist_menu->addSection ("Loaded ADTs");
  ADD_ACTION_NS ( assist_menu
                , "Fix gaps (all loaded ADTs)"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->fixAllGaps();
                  }
                );

  assist_menu->addSection ("Global");
  ADD_ACTION_NS ( assist_menu
                , "Map to big alpha"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->convert_alphamap(true);
                  }
                );
  ADD_ACTION_NS ( assist_menu
                , "Map to old alpha"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->convert_alphamap(false);
                  }
                );

  view_menu->addSection ("Drawing");
  ADD_TOGGLE (view_menu, "Doodads", Qt::Key_F1, _draw_models);
  ADD_TOGGLE (view_menu, "WMO doodads", Qt::Key_F2, _draw_wmo_doodads);
  ADD_TOGGLE (view_menu, "Terrain", Qt::Key_F3, _draw_terrain);
  ADD_TOGGLE (view_menu, "Water", Qt::Key_F4, _draw_water);
  ADD_TOGGLE (view_menu, "WMOs", Qt::Key_F6, _draw_wmo);
  ADD_TOGGLE (view_menu, "Lines", Qt::Key_F7, _draw_lines);
  ADD_TOGGLE (view_menu, "Map contour infos", Qt::Key_F9, _draw_contour);
  ADD_TOGGLE (view_menu, "Wireframe", Qt::Key_F10, _draw_wireframe);
  ADD_TOGGLE (view_menu, "Toggle Animation", Qt::Key_F11, _draw_model_animations);
  ADD_TOGGLE_NS (view_menu, "Flight Bounds", _draw_mfbo);
  ADD_TOGGLE (view_menu, "Hole lines always on", "Shift+F7", _draw_hole_lines);
  ADD_TOGGLE_NS (view_menu, "Models with box", _draw_models_with_box);
  //! \todo space+h in object mode
  ADD_TOGGLE_NS (view_menu, "Draw hidden models", _draw_hidden_models);
  ADD_TOGGLE (view_menu, "Draw fog", Qt::Key_F12, _draw_fog);

  view_menu->addSection ("Windows");
  ADD_TOGGLE (view_menu, "Detail infos", Qt::Key_F8, _show_detail_info_window);
  connect ( &_show_detail_info_window, &noggit::bool_toggle_property::changed
          , guidetailInfos, &QWidget::setVisible
          );
  connect ( guidetailInfos, &noggit::ui::widget::visibilityChanged
          , &_show_detail_info_window, &noggit::bool_toggle_property::set
          );
  ADD_TOGGLE (view_menu, "Minimap", Qt::Key_M, _show_minimap_window);
  connect ( &_show_minimap_window, &noggit::bool_toggle_property::changed
          , _minimap_dock, &QWidget::setVisible
          );
  connect ( _minimap_dock, &QDockWidget::visibilityChanged
          , &_show_minimap_window, &noggit::bool_toggle_property::set
          );
  ADD_TOGGLE (view_menu, "Cursor switcher", "Ctrl+Alt+C", _show_cursor_switcher_window);
  connect ( &_show_cursor_switcher_window, &noggit::bool_toggle_property::changed
          , _cursor_switcher.get(), &QWidget::setVisible
          );
  connect ( _cursor_switcher.get(), &noggit::ui::widget::visibilityChanged
          , &_show_cursor_switcher_window, &noggit::bool_toggle_property::set
          );
  ADD_TOGGLE (view_menu, "Texture palette", Qt::Key_X, _show_texture_palette_window);
  connect ( &_show_texture_palette_window, &noggit::bool_toggle_property::changed
          , TexturePalette, &QWidget::setVisible
          );
  connect ( TexturePalette, &noggit::ui::widget::visibilityChanged
          , &_show_texture_palette_window, &noggit::bool_toggle_property::set
          );

  addHotkey ( Qt::Key_F1
            , MOD_shift
            , [this]
              {
                if (alloff)
                {
                  alloff_models = _draw_models.get();
                  alloff_doodads = _draw_wmo_doodads.get();
                  alloff_contour = _draw_contour.get();
                  alloff_wmo = _draw_wmo.get();
                  alloff_fog = _draw_fog.get();
                  alloff_terrain = _draw_terrain.get();

                  _draw_models.set (false);
                  _draw_wmo_doodads.set (false);
                  _draw_contour.set (true);
                  _draw_wmo.set (false);
                  _draw_terrain.set (true);
                  _draw_fog.set (false);
                }
                else
                {
                  _draw_models.set (alloff_models);
                  _draw_wmo_doodads.set (alloff_doodads);
                  _draw_contour.set (alloff_contour);
                  _draw_wmo.set (alloff_wmo);
                  _draw_terrain.set (alloff_terrain);
                  _draw_fog.set (alloff_fog);
                }
                alloff = !alloff;
              }
            );

  ADD_TOGGLE (help_menu, "Key Bindings", "Ctrl+F1", _show_keybindings_window);
  connect ( &_show_keybindings_window, &noggit::bool_toggle_property::changed
          , _keybindings.get(), &QWidget::setVisible
          );
  connect ( _keybindings.get(), &noggit::ui::widget::visibilityChanged
          , &_show_keybindings_window, &noggit::bool_toggle_property::set
          );

#if defined(_WIN32) || defined(WIN32)
  ADD_ACTION_NS ( help_menu
                , "Manual online"
                , []
                  {
                    ShellExecute ( nullptr
                                 , "open"
                                 , "http://modcraft.superparanoid.de/wiki/index.php5?title=Noggit_user_manual"
                                 , nullptr
                                 , nullptr
                                 , SW_SHOWNORMAL
                                 );
                  }
                );
  ADD_ACTION_NS ( help_menu
                , "Homepage"
                , []
                  {
                    ShellExecute ( nullptr
                                 , "open"
                                 , "http://modcraft.superparanoid.de"
                                 , nullptr
                                 , nullptr
                                 , SW_SHOWNORMAL
                                 );
                  }
                );
#endif

  ADD_ACTION ( file_menu
             , "Add bookmark"
             , Qt::Key_F5
             , [this]
               {
                 std::ofstream f ("bookmarks.txt", std::ios_base::app);
                 f << _world->getMapID() << " "
                   << _camera.position.x << " "
                   << _camera.position.y << " "
                   << _camera.position.z << " "
                   << _camera.yaw()._ << " "
                   << _camera.pitch()._ << " "
                   << _world->getAreaID (_camera.position) << std::endl;
               }
             );

  ADD_ACTION (view_menu, "increase time speed", Qt::Key_N, [this] { mTimespeed += 90.0f; });
  ADD_ACTION (view_menu, "decrease time speed", Qt::Key_B, [this] { mTimespeed = std::max (0.0f, mTimespeed - 90.0f); });
  ADD_ACTION (view_menu, "pause time", Qt::Key_J, [this] { mTimespeed = 0.0f; });

  addHotkey ( Qt::Key_C
            , MOD_ctrl
            , [this]
              {
                objectEditor->copy (*_world->GetCurrentSelection());
              }
            , [this]
              {
                return !!_world->GetCurrentSelection();
              }
            );


  addHotkey ( Qt::Key_C
            , MOD_none
            , [this]
              {
                objectEditor->copy(*_world->GetCurrentSelection());
              }
            , [this] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_C
            , MOD_shift
            , [this]
              {
                cursor_type = ++cursor_type % 4;
              }
            , [this] { return terrainMode != editing_mode::object; }
            );

  addHotkey (Qt::Key_V, MOD_ctrl, [this] { objectEditor->pasteObject (_cursor_pos, _camera.position); });
  addHotkey ( Qt::Key_V
            , MOD_none
            , [this] { objectEditor->pasteObject (_cursor_pos, _camera.position); }
            , [this] { return terrainMode == editing_mode::object; }
            );
  addHotkey ( Qt::Key_V
            , MOD_shift
            , [this] { insert_last_m2_from_wmv(); }
            , [this] { return terrainMode == editing_mode::object; }
            );
  addHotkey ( Qt::Key_V
            , MOD_alt
            , [this] { insert_last_wmo_from_wmv(); }
            , [this] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_C
            , MOD_none
            , [this] { _world->clearVertexSelection(); }
            , [this] { return terrainMode == editing_mode::ground; }
            );

  ADD_ACTION ( view_menu
             , "toggle detail infos"
             , "Ctrl+X"
             , [this]
               {
                 guidetailInfos->setVisible (!guidetailInfos->isVisible());
               }
             );

  ADD_ACTION (view_menu, "invert mouse", "I", [this] { mousedir *= -1.f; });

  ADD_ACTION (view_menu, "decrease camera speed", Qt::Key_O, [this] { _camera.move_speed *= 0.5f; });
  ADD_ACTION (view_menu, "increase camera speed", Qt::Key_P, [this] { _camera.move_speed *= 2.0f; });

  ADD_ACTION (file_menu, "save minimaps", "Ctrl+Shift+P", [this] { Saving = true; });

  ADD_ACTION ( view_menu
             , "turn camera around 180°"
             , Qt::Key_R
             , [this]
               {
                 _camera.add_to_yaw(math::degrees(180.f));
                 _minimap->update();
               }
             );

  ADD_ACTION ( file_menu
             , "write coordinates to port.txt"
             , Qt::Key_G
             , [this]
               {
                 std::ofstream f("ports.txt", std::ios_base::app);
                 f << "Map: " << gAreaDB.getAreaName(_world->getAreaID (_camera.position)) << " on ADT " << std::floor(_camera.position.x / TILESIZE) << " " << std::floor(_camera.position.z / TILESIZE) << std::endl;
                 f << "Trinity:" << std::endl << ".go " << (ZEROPOINT - _camera.position.z) << " " << (ZEROPOINT - _camera.position.x) << " " << _camera.position.y << " " << _world->getMapID() << std::endl;
                 f << "ArcEmu:" << std::endl << ".worldport " << _world->getMapID() << " " << (ZEROPOINT - _camera.position.z) << " " << (ZEROPOINT - _camera.position.x) << " " << _camera.position.y << " " << std::endl << std::endl;
                 f.close();
               }
             );

  addHotkey ( Qt::Key_Y
            , MOD_none
            , [this] { terrainTool->nextType(); }
            , [this] { return terrainMode == editing_mode::ground; }
            );

  addHotkey ( Qt::Key_Y
            , MOD_none
            , [this] { flattenTool->nextFlattenType(); }
            , [this] { return terrainMode == editing_mode::flatten_blur; }
            );

  ADD_ACTION ( view_menu
             , "toggle tile mode"
             , Qt::Key_U
             , [this]
               {
                 if (mViewMode == eViewMode_2D)
                 {
                   mViewMode = eViewMode_3D;
                   set_editing_mode (saveterrainMode);
                 }
                 else
                 {
                   mViewMode = eViewMode_2D;
                   saveterrainMode = terrainMode;
                   set_editing_mode (editing_mode::paint);
                 }
               }
             );

  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                flattenTool->toggleFlattenAngle();
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );

  addHotkey ( Qt::Key_T
            , MOD_space
            , [&]
              {
                flattenTool->nextFlattenMode();
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );

  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                texturingTool->toggle_spray();
              }
            , [&] { return terrainMode == editing_mode::paint; }
            );

  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                _world->setHoleADT (_camera.position, false);
              }
            , [&] { return terrainMode == editing_mode::holes; }
            );

  addHotkey ( Qt::Key_T
            , MOD_alt
            , [&]
              {
                _world->setHoleADT (_camera.position, true);
              }
            , [&] { return terrainMode == editing_mode::holes; }
            );

  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                guiWater->toggle_angled_mode();
              }
            , [&] { return terrainMode == editing_mode::water; }
            );

  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                objectEditor->togglePasteMode();
              }
            , [&] { return terrainMode == editing_mode::object; }
            );


  addHotkey ( Qt::Key_H
            , MOD_none
            , [&]
              {
                if (_world->HasSelection())
                {
                  auto selection = _world->GetCurrentSelection();
                  if (selection->which() == eEntry_Model)
                  {
                    auto&& entity (boost::get<selected_model_type> (*selection)->model.get());
                    auto& hidden (_hidden_models);
                    if (hidden.count (entity))
                    {
                      hidden.erase (entity);
                    }
                    else
                    {
                      hidden.emplace (entity);
                    }
                  }
                  else if (selection->which() == eEntry_WMO)
                  {
                    auto&& entity (boost::get<selected_wmo_type> (*selection)->wmo.get());
                    auto& hidden (_hidden_map_objects);
                    if (hidden.count (entity))
                    {
                      hidden.erase (entity);
                    }
                    else
                    {
                      hidden.emplace (entity);
                    }
                  }
                }
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_H
            , MOD_space
            , [&]
              {
                _draw_hidden_models.set (!_draw_hidden_models.get());
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_H
            , MOD_shift
            , [&]
              {
                _hidden_map_objects.clear();
                _hidden_models.clear();
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_F
            , MOD_space
            , [&]
              {
                terrainTool->flattenVertices();
              }
            , [&] { return terrainMode == editing_mode::ground; }
            );
  addHotkey ( Qt::Key_F
            , MOD_space
            , [&]
              {
                flattenTool->toggleFlattenLock();
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );
  addHotkey ( Qt::Key_F
            , MOD_none
            , [&]
              {
                flattenTool->lockPos (_cursor_pos);
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );
  addHotkey ( Qt::Key_F
            , MOD_space
            , [&]
              {
                guiWater->toggle_lock();
              }
          , [&] { return terrainMode == editing_mode::water; }
          );
  addHotkey( Qt::Key_F
            , MOD_none
            , [&]
              {
                guiWater->lockPos(_cursor_pos);
              }
          , [&] { return terrainMode == editing_mode::water; }
          );
  addHotkey ( Qt::Key_F
            , MOD_none
            , [&]
              {
                if (_world->HasSelection())
                {
                  auto selection = _world->GetCurrentSelection();

                  if (selection->which() == eEntry_Model)
                  {
                    _world->updateTilesModel(boost::get<selected_model_type> (*selection));
                    boost::get<selected_model_type> (*selection)->pos = _cursor_pos;
                    boost::get<selected_model_type> (*selection)->recalcExtents();
                    _world->updateTilesModel(boost::get<selected_model_type> (*selection));
                  }
                  else if (selection->which() == eEntry_WMO)
                  {
                    _world->updateTilesWMO(boost::get<selected_wmo_type> (*selection));
                    boost::get<selected_wmo_type> (*selection)->pos = _cursor_pos;
                    boost::get<selected_wmo_type> (*selection)->recalcExtents();
                    _world->updateTilesWMO(boost::get<selected_wmo_type> (*selection));
                  }
                }
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey (Qt::Key_Plus, MOD_alt, [this] { terrainTool->changeRadius(0.01f); }, [this] { return terrainMode == editing_mode::ground; });

  addHotkey (Qt::Key_Plus, MOD_alt, [this] { flattenTool->changeRadius(0.01f); }, [this] { return terrainMode == editing_mode::flatten_blur; });

  addHotkey ( Qt::Key_Plus
            , MOD_alt
            , [&]
              {
                texturingTool->change_radius(0.1f);
              }
            , [this] { return terrainMode == editing_mode::paint; }
            );

  addHotkey (Qt::Key_Plus, MOD_shift, [this] { _world->fogdistance += 60.0f; });


  addHotkey (Qt::Key_Minus, MOD_alt, [this] { terrainTool->changeRadius(-0.01f); }, [this] { return terrainMode == editing_mode::ground; });

  addHotkey (Qt::Key_Minus, MOD_alt, [this] { flattenTool->changeRadius(-0.01f); }, [this] { return terrainMode == editing_mode::flatten_blur; });

  addHotkey ( Qt::Key_Minus
            , MOD_alt
            , [&]
              {
                texturingTool->change_radius(-0.1f);
              }
            , [this] { return terrainMode == editing_mode::paint; }
            );

  addHotkey (Qt::Key_Minus, MOD_shift, [this] { _world->fogdistance -= 60.0f; });

  addHotkey (Qt::Key_1, MOD_shift, [this] { _camera.move_speed = 15.0f; });
  addHotkey (Qt::Key_2, MOD_shift, [this] { _camera.move_speed = 50.0f; });
  addHotkey (Qt::Key_3, MOD_shift, [this] { _camera.move_speed = 200.0f; });
  addHotkey (Qt::Key_4, MOD_shift, [this] { _camera.move_speed = 800.0f; });
  addHotkey (Qt::Key_1, MOD_alt, [this] { texturingTool->set_brush_level(0.0f); });
  addHotkey (Qt::Key_2, MOD_alt, [this] { texturingTool->set_brush_level(255.0f* 0.25f); });
  addHotkey (Qt::Key_3, MOD_alt, [this] { texturingTool->set_brush_level(255.0f* 0.5f); });
  addHotkey (Qt::Key_4, MOD_alt, [this] { texturingTool->set_brush_level(255.0f* 0.75f); });
  addHotkey (Qt::Key_5, MOD_alt, [this] { texturingTool->set_brush_level(255.0f); });

  addHotkey (Qt::Key_1, MOD_none, [this] { set_editing_mode (editing_mode::ground); });
  addHotkey (Qt::Key_2, MOD_none, [this] { set_editing_mode (editing_mode::flatten_blur); });
  addHotkey (Qt::Key_3, MOD_none, [this] { set_editing_mode (editing_mode::paint); });
  addHotkey (Qt::Key_4, MOD_none, [this] { set_editing_mode (editing_mode::holes); });
  addHotkey (Qt::Key_5, MOD_none, [this] { set_editing_mode (editing_mode::areaid); });
  addHotkey (Qt::Key_6, MOD_none, [this] { set_editing_mode (editing_mode::flags); });
  addHotkey (Qt::Key_7, MOD_none, [this] { set_editing_mode (editing_mode::water); });
  addHotkey (Qt::Key_8, MOD_none, [this] { set_editing_mode (editing_mode::mccv); });
  addHotkey (Qt::Key_9, MOD_none, [this] { set_editing_mode (editing_mode::object); });

  addHotkey (Qt::Key_0, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 0; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_1, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 1; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_2, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 2; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_3, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 3; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_4, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 4; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_5, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 5; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_6, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 6; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_7, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 7; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_8, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 8; }, [this] { return _world->IsSelection(eEntry_WMO); });
  addHotkey (Qt::Key_9, MOD_ctrl, [this] { boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->doodadset = 9; }, [this] { return _world->IsSelection(eEntry_WMO); });

  // Water unable to save warning
  waterSaveWarning = new noggit::ui::water_save_warning;
  waterSaveWarning->hide();

  // modelimport
  objectEditor->modelImport = new noggit::ui::model_import(this);

  // helper models
  HelperModels = new noggit::ui::helper_models(this);
}

MapView::MapView( math::degrees camera_yaw0
                , math::degrees camera_pitch0
                , math::vector_3d camera_pos
                , noggit::ui::main_window* main_window
                , World* world
                )
  : _camera (camera_pos, camera_yaw0, camera_pitch0)
  , mTimespeed(0.0f)
  , _main_window (main_window)
  , _world (world)
  , _status_position (new QLabel (this))
  , _status_selection (new QLabel (this))
  , _status_area (new QLabel (this))
  , _status_time (new QLabel (this))
  , _minimap (new noggit::ui::minimap_widget (nullptr))
  , _minimap_dock (new QDockWidget ("Minimap", this))
  , _cursor_switcher (new noggit::ui::cursor_switcher (cursor_color, cursor_type))
  , _keybindings (new noggit::ui::help)
{
  _main_window->statusBar()->addWidget (_status_position);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_position); }
          );
  _main_window->statusBar()->addWidget (_status_selection);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_selection); }
          );
  _main_window->statusBar()->addWidget (_status_area);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_area); }
          );
  _main_window->statusBar()->addWidget (_status_time);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_time); }
          );

  _minimap->world (_world);
  _minimap->camera (&_camera);
  _minimap->draw_skies (true);
  _minimap->draw_boundaries (true);

  connect ( _minimap, &noggit::ui::minimap_widget::map_clicked
          , [this] (World* world, math::vector_3d const& pos)
            {
              move_camera_with_auto_height (pos);
            }
          );

  _minimap_dock->setFeatures ( QDockWidget::DockWidgetMovable
                             | QDockWidget::DockWidgetFloatable
                             | QDockWidget::DockWidgetClosable
                             );
  _minimap_dock->setWidget (_minimap);
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _minimap_dock);
  _minimap_dock->setVisible (false);


  setWindowTitle ("Noggit Studio - " STRPRODUCTVER);

  // load cursor settings
  if (boost::filesystem::exists("noggit.conf"))
  {
    ConfigFile myConfigfile = ConfigFile("noggit.conf");
    if (myConfigfile.keyExists("RedColor") && myConfigfile.keyExists("GreenColor") && myConfigfile.keyExists("BlueColor") && myConfigfile.keyExists("AlphaColor"))
    {
      cursor_color.x = myConfigfile.read<float>("RedColor");
      cursor_color.y = myConfigfile.read<float>("GreenColor");
      cursor_color.z = myConfigfile.read<float>("BlueColor");
      cursor_color.w = myConfigfile.read<float>("AlphaColor");
    }

    if (myConfigfile.keyExists("CursorType"))
    {
      cursor_type = myConfigfile.read<int>("CursorType");
    }
  }

  setFocusPolicy (Qt::StrongFocus);
  setMouseTracking (true);

  moving = strafing = updown = lookat = turn = 0.0f;

  mousedir = -1.0f;

  lastBrushUpdate = 0;

  look = false;
  mViewMode = eViewMode_3D;

  init_tablet();

  _startup_time.start();
  _update_every_event_loop.start (0);
  connect (&_update_every_event_loop, &QTimer::timeout, [this] { update(); });
}

  void MapView::move_camera_with_auto_height (math::vector_3d const& pos)
  {
    makeCurrent();
    opengl::context::scoped_setter const _ (::gl, context());

    if (!_world->mapIndex.tileLoaded (pos))
    {
      _world->mapIndex.loadTile (pos);
    }

    _camera.position = pos;
    _camera.position.y = 0.0f;

    _world->GetVertex (pos.x, pos.z, &_camera.position);

    // min elevation according to https://wowdev.wiki/AreaTable.dbc
    //! \ todo use the current area's MinElevation
    if (_camera.position.y < -5000.0f)
    {
      //! \todo use the height of a model/wmo of the tile (or the map) ?
      _camera.position.y = 0.0f;
    }

    _camera.position.y += 50.0f;

    _minimap->update();
  }

  void MapView::initializeGL()
  {
    opengl::context::scoped_setter const _ (::gl, context());
    gl.viewport(0.0f, 0.0f, width(), height());

    gl.clearColor (0.0f, 0.0f, 0.0f, 1.0f);

    createGUI();

  set_editing_mode (editing_mode::ground);

    move_camera_with_auto_height (_camera.position);

    gl.enableClientState (GL_VERTEX_ARRAY);
    gl.enableClientState (GL_NORMAL_ARRAY);
    gl.enableClientState (GL_TEXTURE_COORD_ARRAY);
  }

  void MapView::paintGL()
  {
    {
      makeCurrent();
      opengl::context::scoped_setter const _ (::gl, context());
      const qreal now(_startup_time.elapsed() / 1000.0);
      tick (now - _last_update);
      _last_update = now;
    }

    {
      makeCurrent();
      opengl::context::scoped_setter const _ (::gl, context());
      gl.clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      //! \todo  Get this out or do it somehow else. This is ugly and is a senseless if each draw.
      if (Saving)
      {
        gl.matrixMode (GL_PROJECTION);
        gl.loadIdentity();
        gl.ortho
          (-2.0f * aspect_ratio(), 2.0f * aspect_ratio(), 2.0f, -2.0f, -100.0f, 300.0f);
        gl.matrixMode (GL_MODELVIEW);
        gl.loadIdentity();

        _world->saveMap (width(), height());
        Saving = false;
      }

      switch (mViewMode)
      {
      case eViewMode_2D:
        displayViewMode_2D();
        break;

      case eViewMode_3D:
        displayViewMode_3D();
        break;
      }
    }
  }

  void MapView::resizeGL (int width, int height)
  {
    opengl::context::scoped_setter const _ (::gl, context());
    gl.viewport(0.0f, 0.0f, width, height);
  }


MapView::~MapView()
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  delete _world;
  _world = nullptr;
}

void MapView::tick (float dt)
{
#ifdef _WIN32
  if (_tablet_active && Settings::getInstance()->tabletMode)
  {
    PACKET pkt;
    while (gpWTPacketsGet(hCtx, 1, &pkt) > 0) //this is a while because we really only want the last packet.
    {
      _tablet_pressure = pkt.pkNormalPressure;
    }
  }
#endif

  // start unloading tiles
  _world->mapIndex.enterTile (tile_index (_camera.position));
  _world->mapIndex.unloadTiles (tile_index (_camera.position));

  dt = std::min(dt, 1.0f);

  update_cursor_pos();

#ifdef _WIN32
  if (_tablet_active && Settings::getInstance()->tabletMode)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      terrainTool->setRadius (_tablet_pressure / 20.0f);
    case editing_mode::flatten_blur:
      flattenTool->setRadius (_tablet_pressure / 20.0f);
      break;
    case editing_mode::paint:
      texturingTool->change_pressure (_tablet_pressure / 2048.0f);
      break;
    }
  }
#endif

    math::degrees yaw (-_camera.yaw()._);

    math::vector_3d dir(1.0f, 0.0f, 0.0f);
    math::vector_3d dirUp(1.0f, 0.0f, 0.0f);
    math::vector_3d dirRight(0.0f, 0.0f, 1.0f);
    math::rotate(0.0f, 0.0f, &dir.x, &dir.y, _camera.pitch());
    math::rotate(0.0f, 0.0f, &dir.x, &dir.z, yaw);

    if (_mod_ctrl_down)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.y, _camera.pitch());
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.y, _camera.pitch());
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, yaw);
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z,yaw);
    }
    else if(!_mod_shift_down)
    {
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, yaw);
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, yaw);
    }

    auto Selection = _world->GetCurrentSelection();
    if (Selection)
    {
      // update rotation editor if the selection has changed
      if (!lastSelected || lastSelected != Selection)
      {
        objectEditor->rotationEditor->select(*Selection);
      }

      bool canMoveObj = !objectEditor->rotationEditor->hasFocus();

      // Set move scale and rotate for numpad keys
      if (_mod_ctrl_down && _mod_shift_down)  moveratio = 0.1f;
      else if (_mod_shift_down) moveratio = 0.01f;
      else if (_mod_ctrl_down) moveratio = 0.005f;
      else moveratio = 0.001f;

      if (canMoveObj && (keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0))
      {
        // Move scale and rotate with numpad keys
        if (Selection->which() == eEntry_WMO)
        {
          _world->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          boost::get<selected_wmo_type> (*Selection)->pos.x += keyx * moveratio;
          boost::get<selected_wmo_type> (*Selection)->pos.y += keyy * moveratio;
          boost::get<selected_wmo_type> (*Selection)->pos.z += keyz * moveratio;
          boost::get<selected_wmo_type> (*Selection)->dir.y += keyr * moveratio * 5;

          boost::get<selected_wmo_type> (*Selection)->recalcExtents();
          _world->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          objectEditor->rotationEditor->updateValues();
        }

        if (Selection->which() == eEntry_Model)
        {
          _world->updateTilesModel(boost::get<selected_model_type> (*Selection));
          boost::get<selected_model_type> (*Selection)->pos.x += keyx * moveratio;
          boost::get<selected_model_type> (*Selection)->pos.y += keyy * moveratio;
          boost::get<selected_model_type> (*Selection)->pos.z += keyz * moveratio;
          boost::get<selected_model_type> (*Selection)->dir.y += keyr * moveratio * 5;
          boost::get<selected_model_type> (*Selection)->sc += keys * moveratio / 50;
          boost::get<selected_model_type> (*Selection)->recalcExtents();
          _world->updateTilesModel(boost::get<selected_model_type> (*Selection));
          objectEditor->rotationEditor->updateValues();
        }
      }

      math::vector_3d ObjPos;
      if (_world->IsSelection(eEntry_Model))
      {
        //! \todo  Tell me what this is.
        ObjPos = boost::get<selected_model_type> (*Selection)->pos - _camera.position;
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.y, _camera.pitch());
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.z, yaw);
        ObjPos.x = std::abs(ObjPos.x);
      }

      // moving and scaling objects
      //! \todo  Alternatively automatically align it to the terrain.
      if (MoveObj && canMoveObj)
      {
        ObjPos.x = 80.0f;
        if (Selection->which() == eEntry_WMO)
        {
          _world->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));

          if (_mod_shift_down)
          {
            boost::get<selected_wmo_type> (*Selection)->pos.y += mv * ObjPos.x;
          }
          else
          {
            if (Environment::getInstance()->moveModelToCursorPos)
            {
              boost::get<selected_wmo_type> (*Selection)->pos.x = _cursor_pos.x - objMoveOffset.x;
              boost::get<selected_wmo_type> (*Selection)->pos.z = _cursor_pos.z - objMoveOffset.z;
            }
            else
            {
              boost::get<selected_wmo_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
              boost::get<selected_wmo_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
            }
          }

          boost::get<selected_wmo_type> (*Selection)->recalcExtents();
          _world->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          objectEditor->rotationEditor->updateValues();
        }
        else if (Selection->which() == eEntry_Model)
        {
          _world->updateTilesModel(boost::get<selected_model_type> (*Selection));
          if (_mod_alt_down)
          {
            float ScaleAmount = pow(2.0f, mv * 4.0f);

            boost::get<selected_model_type> (*Selection)->sc *= ScaleAmount;
            if (boost::get<selected_model_type> (*Selection)->sc > 63.9f)
              boost::get<selected_model_type> (*Selection)->sc = 63.9f;
            else if (boost::get<selected_model_type> (*Selection)->sc < 0.00098f)
              boost::get<selected_model_type> (*Selection)->sc = 0.00098f;
          }
          else
          {
            if (_mod_shift_down)
            {
              boost::get<selected_model_type> (*Selection)->pos.y += mv * ObjPos.x;
            }
            else
            {
              if (Environment::getInstance()->moveModelToCursorPos)
              {
                boost::get<selected_model_type> (*Selection)->pos.x = _cursor_pos.x - objMoveOffset.x;
                boost::get<selected_model_type> (*Selection)->pos.z = _cursor_pos.z - objMoveOffset.z;
              }
              else
              {
                boost::get<selected_model_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
                boost::get<selected_model_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
              }

            }
          }

          objectEditor->rotationEditor->updateValues();
          boost::get<selected_model_type> (*Selection)->recalcExtents();
          _world->updateTilesModel(boost::get<selected_model_type> (*Selection));
        }
      }


      // rotating objects
      if (look && canMoveObj)
      {
        float * lTarget = nullptr;
        bool lModify = false;

        if (Selection->which() == eEntry_Model)
        {
          lModify = _mod_shift_down | _mod_ctrl_down | _mod_alt_down;
          if (_mod_shift_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.y;
          else if (_mod_ctrl_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.x;
          else if (_mod_alt_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.z;

        }
        else if (Selection->which() == eEntry_WMO)
        {
          lModify = _mod_shift_down | _mod_ctrl_down | _mod_alt_down;
          if (_mod_shift_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.y;
          else if (_mod_ctrl_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.x;
          else if (_mod_alt_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.z;

        }

        if (lModify && lTarget)
        {
          _world->updateTilesEntry(*Selection);

          *lTarget = *lTarget + rh + rv;

          if (*lTarget > 360.0f)
            *lTarget = *lTarget - 360.0f;
          else if (*lTarget < -360.0f)
            *lTarget = *lTarget + 360.0f;

          objectEditor->rotationEditor->updateValues();

          if (Selection->which() == eEntry_WMO)
          {
            boost::get<selected_wmo_type> (*Selection)->recalcExtents();
            _world->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          }
          else if (Selection->which() == eEntry_Model)
          {
            boost::get<selected_model_type> (*Selection)->recalcExtents();
            _world->updateTilesModel(boost::get<selected_model_type> (*Selection));
          }
        }
      }

      mh = 0;
      mv = 0;
      rh = 0;
      rv = 0;


      if (leftMouse && Selection->which() == eEntry_MapChunk)
      {
        bool underMap = _world->isUnderMap(_cursor_pos);

        switch (terrainMode)
        {
        case editing_mode::ground:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              terrainTool->changeTerrain(_cursor_pos, 7.5f * dt);
            }
            else if (_mod_ctrl_down)
            {
              terrainTool->changeTerrain(_cursor_pos, -7.5f * dt);
            }
          }
          break;
        case editing_mode::flatten_blur:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              flattenTool->flatten(_cursor_pos, dt);
            }
            else if (_mod_ctrl_down)
            {
              flattenTool->blur(_cursor_pos, dt);
            }
          }
          break;
        case editing_mode::paint:
          if (_mod_shift_down && _mod_ctrl_down && _mod_alt_down)
          {
            // clear chunk texture
            if (mViewMode == eViewMode_3D && !underMap)
              _world->eraseTextures(_cursor_pos);
            else if (mViewMode == eViewMode_2D)
              _world->eraseTextures({CHUNKSIZE * 4.0f * aspect_ratio() * (static_cast<float>(_last_mouse_pos.x()) / static_cast<float>(width()) - 0.5f) / _2d_zoom + _camera.position.x, 0.f, CHUNKSIZE * 4.0f * (static_cast<float>(_last_mouse_pos.y()) / static_cast<float>(height()) - 0.5f) / _2d_zoom + _camera.position.z});
          }
          else if (_mod_ctrl_down)
          {
            // Pick texture
            TexturePicker->getTextures(*_world->GetCurrentSelection());
          }
          else  if (_mod_shift_down && !!noggit::ui::selected_texture::get())
          {
            if (mViewMode == eViewMode_3D && !underMap)
            {
              texturingTool->paint(_cursor_pos, dt, *noggit::ui::selected_texture::get());
            }
            else if (mViewMode == eViewMode_2D)
            {
              math::vector_3d pos( CHUNKSIZE * 4.0f * aspect_ratio() * ((float)_last_mouse_pos.x() / (float)width() - 0.5f ) / _2d_zoom
                                  , 0.0f
                                  , CHUNKSIZE * 4.0f * ((float)_last_mouse_pos.y() / (float)height() - 0.5f) / _2d_zoom
                                  );

              pos += _camera.position;
              texturingTool->paint(pos, dt, *noggit::ui::selected_texture::get());
            }
          }
          break;

        case editing_mode::holes:
          if (mViewMode == eViewMode_3D)
          {
            // no undermap check here, else it's impossible to remove holes
            if (_mod_shift_down)
            {
              auto pos (boost::get<selected_chunk_type> (*Selection).position);
              _world->setHole(pos, _mod_alt_down, false);
            }
            else if (_mod_ctrl_down && !underMap)
            {
              _world->setHole(_cursor_pos, _mod_alt_down, true);
            }
          }
          break;
        case editing_mode::areaid:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              // draw the selected AreaId on current selected chunk
              _world->setAreaID(_cursor_pos, _selected_area_id, false);
            }
            else if (_mod_ctrl_down)
            {
              // pick areaID from chunk
              MapChunk* chnk (boost::get<selected_chunk_type> (*_world->GetCurrentSelection()).chunk);
              int newID = chnk->getAreaID();
              _selected_area_id = newID;
              ZoneIDBrowser->setZoneID(newID);
            }
          }
          break;
        case editing_mode::flags:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              _world->mapIndex.setFlag(true, _cursor_pos, FLAG_IMPASS);
            }
            else if (_mod_ctrl_down)
            {
              _world->mapIndex.setFlag(false, _cursor_pos, FLAG_IMPASS);
            }
          }
          break;
        case editing_mode::water:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              guiWater->paintLiquid(_cursor_pos, true);
            }
            else if (_mod_ctrl_down)
            {
              guiWater->paintLiquid(_cursor_pos, false);
            }
          }
          break;
        case editing_mode::mccv:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              shaderTool->changeShader(_cursor_pos, dt, true);
            }
            if (_mod_ctrl_down)
            {
              shaderTool->changeShader(_cursor_pos, dt, false);
            }
          }
          break;
        }
      }
    }

    if (mViewMode != eViewMode_2D)
    {
      if (turn)
      {
        _camera.add_to_yaw(math::degrees(turn));
      }
      if (lookat)
      {
        _camera.add_to_pitch(math::degrees(lookat));
      }
      if (moving)
      {
        _camera.move_forward(moving, dt);
      }
      if (strafing)
      {
        _camera.move_horizontal(strafing, dt);
      }
      if (updown)
      {
        _camera.move_vertical(updown, dt);
      }
      _minimap->update();
    }
    else
    {
      //! \todo this is total bullshit. there should be a seperate view and camera class for tilemode
      if (moving)
        _camera.position.z -= dt * _camera.move_speed * moving / (_2d_zoom * 1.5f);
      if (strafing)
        _camera.position.x += dt * _camera.move_speed * strafing / (_2d_zoom * 1.5f);
      if (updown)
        _2d_zoom *= pow(2.0f, dt * updown * 4.0f);

      _2d_zoom = std::min(std::max(_2d_zoom, 0.1f), 2.0f);
    }

  _world->time += this->mTimespeed * dt;


  _world->animtime += dt * 1000.0f;
  globalTime = static_cast<int>(_world->animtime);

  _world->tick (dt);

  lastSelected = _world->GetCurrentSelection();

  QString status;
  status += ( QString ("tile: %1 %2")
            . arg (std::floor (_camera.position.x / TILESIZE))
            . arg (std::floor (_camera.position.z / TILESIZE))
            );
  status += ( QString ("; coordinates client: (%1, %2, %3), server: (%4, %5, %6)")
            . arg (_camera.position.x)
            . arg (_camera.position.z)
            . arg (_camera.position.y)
            . arg (ZEROPOINT - _camera.position.z)
            . arg (ZEROPOINT - _camera.position.x)
            . arg (_camera.position.y)
            );

  _status_position->setText (status);

  auto lSelection = _world->GetCurrentSelection();
  if (!lSelection)
  {
    _status_selection->setText ("");
  }
  else
  {
    switch (lSelection->which())
    {
    case eEntry_Model:
      {
        auto instance (boost::get<selected_model_type> (*lSelection));
        _status_selection->setText
          ( QString ("%1: %2")
          . arg (instance->d1)
          . arg (QString::fromStdString (instance->model->_filename))
          );
        break;
      }
    case eEntry_WMO:
      {
        auto instance (boost::get<selected_wmo_type> (*lSelection));
        _status_selection->setText
          ( QString ("%1: %2")
          . arg (instance->mUniqueID)
          . arg (QString::fromStdString (instance->wmo->_filename))
          );
        break;
      }
    case eEntry_MapChunk:
      {
        auto chunk (boost::get<selected_chunk_type> (*lSelection).chunk);
        _status_selection->setText
          (QString ("%1, %2").arg (chunk->px).arg (chunk->py));
        break;
      }
    }
  }

  _status_area->setText
    (QString::fromStdString (gAreaDB.getAreaName (_world->getAreaID (_camera.position))));

  {
    int time ((static_cast<int>(_world->time) % 2880) / 2);
    std::stringstream timestrs;
    timestrs << "Time: " << (time / 60) << ":" << std::setfill ('0')
             << std::setw (2) << (time % 60);

#ifdef _WIN32
    if (_tablet_active && Settings::getInstance()->tabletMode)
    {
      timestrs << ", Pres: " << _tablet_pressure;
    }
#endif

    _status_time->setText (QString::fromStdString (timestrs.str()));
  }

  guiWater->updatePos (_camera.position);

  {
    auto lSelection = gWorld->GetCurrentSelection();
    std::stringstream detailInfo;
    if (lSelection)
    {
      switch (lSelection->which())
      {
      case eEntry_Model:
        {
          auto instance (boost::get<selected_model_type> (*lSelection));
          detailInfo << "filename: " << instance->model->_filename
                     << "\nunique ID: " << instance->d1
                     << "\nposition X/Y/Z: " << instance->pos.x << " / " << instance->pos.y << " / " << instance->pos.z
                     << "\nrotation X/Y/Z: " << instance->dir.x << " / " << instance->dir.y << " / " << instance->dir.z
                     << "\nscale: " << instance->sc
                     << "\ntextures Used: " << instance->model->header.nTextures;

          for (unsigned int j = 0; j < std::min(instance->model->header.nTextures, 6U); j++)
          {
            detailInfo << "\n " << (j + 1) << ": " << instance->model->_textures[j]->filename();
          }
          if (instance->model->header.nTextures > 25)
          {
            detailInfo << "\n and more.";
          }

          detailInfo << "\n";
          break;
        }
      case eEntry_WMO:
        {
          auto instance (boost::get<selected_wmo_type> (*lSelection));
          detailInfo << "filename: " << instance->wmo->_filename
                     << "\nunique ID: " << instance->mUniqueID
                     << "\nposition X/Y/Z: " << instance->pos.x << " / " << instance->pos.y << " / " << instance->pos.z
                     << "\nrotation X/Y/Z: " << instance->dir.x << " / " << instance->dir.y << " / " << instance->dir.z
                     << "\ndoodad set: " << instance->doodadset
                     << "\ntextures used: " << instance->wmo->textures.size();


          const unsigned int texture_count (std::min((unsigned int)(instance->wmo->textures.size()), 8U));
          for (unsigned int j = 0; j < texture_count; j++)
          {
            detailInfo << "\n " << (j + 1) << ": " << instance->wmo->textures[j];
          }
          if (instance->wmo->textures.size() > 25)
          {
            detailInfo << "\n and more.";
          }

          detailInfo << "\n";
          break;
        }
      case eEntry_MapChunk:
        {
          auto chunk (boost::get<selected_chunk_type> (*lSelection).chunk);
          int flags = chunk->Flags;

          detailInfo << "MCNK " << chunk->px << ", " << chunk->py << " (" << chunk->py * 16 + chunk->px
                     << ") of tile (" << chunk->mt->index.x << " " << chunk->mt->index.z << ")"
                     << "\narea ID: " << chunk->getAreaID() << " (\"" << gAreaDB.getAreaName(chunk->getAreaID()) << "\")"
                     << "\nflags: "
                     << (flags & FLAG_SHADOW ? "shadows " : "")
                     << (flags & FLAG_IMPASS ? "impassable " : "")
                     << (flags & FLAG_LQ_RIVER ? "river " : "")
                     << (flags & FLAG_LQ_OCEAN ? "ocean " : "")
                     << (flags & FLAG_LQ_MAGMA ? "lava" : "")
                     << "\ntextures used: " << chunk->_texture_set.num();

          //! \todo get a list of textures and their flags as well as detail doodads.
          /*
            for( int q = 0; q < chunk->nTextures; q++ )
            {
            //s << " ";
            //s "  Flags - " << chunk->texFlags[q] << " Effect ID - " << chunk->effectID[q] << std::endl;

            if( chunk->effectID[q] != 0 )
            for( int r = 0; r < 4; r++ )
            {
            const char *EffectModel = getGroundEffectDoodad( chunk->effectID[q], r );
            if( EffectModel )
            {
            s << r << " - World\\NoDXT\\" << EffectModel << endl;
            //freetype::shprint( app.getArial16(), 30, 103 + TextOffset, "%d - World\\NoDXT\\%s", r, EffectModel );
            TextOffset += 20;
            }
            }

            }
          */

          detailInfo << "\n";

          break;
        }
      }
    }

    guidetailInfos->setText(detailInfo.str());
  }
}

math::vector_4d MapView::normalized_device_coords (int x, int y) const
{
  return {2.0f * x / width() - 1.0f, 1.0f - 2.0f * y / height(), 0.0f, 1.0f};
}

float MapView::aspect_ratio() const
{
  return float (width()) / float (height());
}

selection_result MapView::intersect_result(bool terrain_only)
{
  // during rendering we multiply perspective * view
  // so we need the same order here and then invert.
  math::vector_3d const pos
    ( ( ( math::perspective ( _camera.fov()
                            , aspect_ratio()
                            , 1.f
                            , Settings::getInstance()->FarZ
                            )
        * math::look_at ( _camera.position
                        , _camera.look_at()
                        , { 0.0f, 1.0f, 0.0f }
                        )
        ).inverted()
      * normalized_device_coords (_last_mouse_pos.x(), _last_mouse_pos.y())
      ).xyz_normalized_by_w()
    );

  math::ray ray (_camera.position, pos - _camera.position);

  selection_result results
    ( _world->intersect ( ray
                        , terrain_only
                        , terrainMode == editing_mode::object
                        , _draw_terrain.get()
                        , _draw_wmo.get()
                        , _draw_models.get()
                        , _draw_hidden_models.get() ? std::unordered_set<WMO*>() : _hidden_map_objects
                        , _draw_hidden_models.get() ? std::unordered_set<Model*>() : _hidden_models
                        )
    );

  std::sort ( results.begin()
            , results.end()
            , [] (selection_entry const& lhs, selection_entry const& rhs)
              {
                return lhs.first < rhs.first;
              }
            );

  return results;
}

void MapView::doSelection (bool selectTerrainOnly)
{
  selection_result results(intersect_result(selectTerrainOnly));

  if (results.empty())
  {
    _world->SetCurrentSelection (boost::none);
  }
  else
  {
    auto const& hit (results.front().second);
    _world->SetCurrentSelection (hit);

    _cursor_pos = hit.which() == eEntry_Model ? boost::get<selected_model_type>(hit)->pos
      : hit.which() == eEntry_WMO ? boost::get<selected_wmo_type>(hit)->pos
      : hit.which() == eEntry_MapChunk ? boost::get<selected_chunk_type>(hit).position
      : throw std::logic_error("bad variant");
  }
}

void MapView::update_cursor_pos()
{
  selection_result results (intersect_result (true));

  if (!results.empty())
  {
    auto const& hit(results.front().second);
    // hit cannot be something else than a chunk
    _cursor_pos = boost::get<selected_chunk_type>(hit).position;
  }
}

void MapView::displayViewMode_2D()
{
  gl.matrixMode (GL_PROJECTION);
  gl.loadIdentity();
  gl.ortho
    (-2.0f * aspect_ratio(), 2.0f * aspect_ratio(), 2.0f, -2.0f, -100.0f, 300.0f);
  gl.matrixMode (GL_MODELVIEW);
  gl.loadIdentity();

  _world->drawTileMode ( _camera.yaw()._
                       , _camera.position
                       , _draw_lines.get()
                       , _2d_zoom
                       , aspect_ratio()
                       );


  const float mX = (CHUNKSIZE * 4.0f * aspect_ratio() * (static_cast<float>(_last_mouse_pos.x()) / static_cast<float>(width()) - 0.5f) / _2d_zoom + _camera.position.x) / CHUNKSIZE;
  const float mY = (CHUNKSIZE * 4.0f * (static_cast<float>(_last_mouse_pos.y()) / static_cast<float>(height()) - 0.5f) / _2d_zoom + _camera.position.z) / CHUNKSIZE;

  // draw brush
  {
    opengl::scoped::matrix_pusher const matrix;

    gl.scalef(_2d_zoom, _2d_zoom, 1.0f);
    gl.translatef(-_camera.position.x / CHUNKSIZE, -_camera.position.z / CHUNKSIZE, 0);

    gl.color4f(1.0f, 1.0f, 1.0f, 0.5f);
    opengl::texture::set_active_texture(1);
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture(0);
    opengl::texture::enable_texture();

    opengl::texture brush_texture;

    {
      char tex[256 * 256];

      float const change = 2.0f / 256.0f;
      float const hardness (texturingTool->texture_brush().getHardness());

      float y = -1;
      for (int j = 0; j < 256; j++)
      {
        float x = -1;
        for (int i = 0; i < 256; ++i)
        {
          float dist = std::sqrt (x * x + y * y);
          if (dist > 1)
            tex[j * 256 + i] = 0;
          else if (dist < hardness)
            tex[j * 256 + i] = (unsigned char)255;
          else
            tex[j * 256 + i] = (unsigned char)(255.0f * (1 - (dist - hardness) / (1 - hardness)) + 0.5f);

          x += change;
        }
        y += change;
      }
      brush_texture.bind();
      gl.texImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 256, 256, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tex);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    const float tRadius = texturingTool->brush_radius() / CHUNKSIZE;// *_2d_zoom;
    gl.begin(GL_QUADS);
    gl.texCoord2f(0.0f, 0.0f);
    gl.vertex3f(mX - tRadius, mY + tRadius, 0);
    gl.texCoord2f(1.0f, 0.0f);
    gl.vertex3f(mX + tRadius, mY + tRadius, 0);
    gl.texCoord2f(1.0f, 1.0f);
    gl.vertex3f(mX + tRadius, mY - tRadius, 0);
    gl.texCoord2f(0.0f, 1.0f);
    gl.vertex3f(mX - tRadius, mY - tRadius, 0);
    gl.end();
  }
}

void MapView::displayViewMode_3D()
{
  //! \note Select terrain below mouse, if no item selected or the item is map.
  if (!_world->IsSelection(eEntry_Model) && !_world->IsSelection(eEntry_WMO) && _auto_selecting_mode.get())
  {
    doSelection(true);
  }

  //! \ todo: make the current tool return the radius
  float radius = 0.0f, hardness = 0.0f, inner_radius = 0.0f, angle = 0.0f, orientation = 0.0f;
  math::vector_3d ref_pos;
  bool angled_mode = false, use_ref_pos = false;

  switch (terrainMode)
  {
  case editing_mode::ground:
    radius = terrainTool->brushRadius();
    inner_radius = terrainTool->innerRadius();
    break;
  case editing_mode::flatten_blur:
    radius = flattenTool->brushRadius();
    angle = flattenTool->angle();
    orientation = flattenTool->orientation();
    ref_pos = flattenTool->ref_pos();
    angled_mode = flattenTool->angled_mode();
    use_ref_pos = flattenTool->use_ref_pos();
    break;
  case editing_mode::paint:
    radius = texturingTool->brush_radius();
    hardness = texturingTool->hardness();
    break;
  case editing_mode::water:
    radius = guiWater->brushRadius();
    angle = guiWater->angle();
    orientation = guiWater->orientation();
    ref_pos = guiWater->ref_pos();
    angled_mode = guiWater->angled_mode();
    use_ref_pos = guiWater->use_ref_pos();
    break;
  case editing_mode::mccv:
    radius = shaderTool->brushRadius();
    break;
  }

  gl.matrixMode (GL_PROJECTION);
  gl.loadIdentity();
  opengl::matrix::perspective
    (_camera.fov(), aspect_ratio(), 1.f, Settings::getInstance()->FarZ);
  gl.matrixMode (GL_MODELVIEW);
  gl.loadIdentity();
  opengl::matrix::look_at
    (_camera.position, _camera.look_at(), {0.0f, 1.0f, 0.0f});

  _world->draw ( _cursor_pos
               , cursor_color
               , cursor_type
               , radius
               , hardness
               , texturingTool->show_unpaintable_chunks()
               , _draw_contour.get()
               , inner_radius
               , ref_pos
               , angle
               , orientation
               , use_ref_pos
               , angled_mode
               , terrainMode == editing_mode::paint
               , terrainMode == editing_mode::flags
               , terrainMode == editing_mode::water
               , terrainMode == editing_mode::areaid
               , terrainMode
               , _camera.position
               , _draw_mfbo.get()
               , _draw_wireframe.get()
               , _draw_lines.get()
               , _draw_terrain.get()
               , _draw_wmo.get()
               , _draw_water.get()
               , _draw_wmo_doodads.get()
               , _draw_models.get()
               , _draw_model_animations.get()
               , _draw_hole_lines.get() || terrainMode == editing_mode::holes
               , _draw_models_with_box.get()
               , _draw_hidden_models.get() ? std::unordered_set<WMO*>() : _hidden_map_objects
               , _draw_hidden_models.get() ? std::unordered_set<Model*>() : _hidden_models
               , _area_id_colors
               , _draw_fog.get()
               );
}

void MapView::keyPressEvent (QKeyEvent *event)
{
  size_t const modifier
    ( ((event->modifiers() & Qt::ShiftModifier) ? MOD_shift : 0)
    | ((event->modifiers() & Qt::ControlModifier) ? MOD_ctrl : 0)
    | ((event->modifiers() & Qt::AltModifier) ? MOD_alt : 0)
    | ((event->modifiers() & Qt::MetaModifier) ? MOD_meta : 0)
    | (_mod_space_down ? MOD_space : 0)
    );

  for (auto&& hotkey : hotkeys)
  {
    if (event->key() == hotkey.key && modifier == hotkey.modifiers && hotkey.condition())
    {
      makeCurrent();
      opengl::context::scoped_setter const _ (::gl, context());

      hotkey.function();

      return;
    }
  }

  if (event->key() == Qt::Key_Shift)
    _mod_shift_down = true;

  if (event->key() == Qt::Key_Alt)
    _mod_alt_down = true;

  if (event->key() == Qt::Key_Control)
    _mod_ctrl_down = true;

  if (event->key() == Qt::Key_Space)
    _mod_space_down = true;

  // movement
  if (event->key() == Qt::Key_W)
  {
    key_w = true;
    moving += 1.0f;
  }
  if (event->key() == Qt::Key_S)
  {
    moving -= 1.0f;
  }

  if (event->key() == Qt::Key_Up)
  {
    lookat += 0.75f;
  }
  if (event->key() == Qt::Key_Down)
  {
    lookat -= 0.75f;
  }

  if (event->key() == Qt::Key_Right)
  {
    turn += 0.75f;
  }
  if (event->key() == Qt::Key_Left)
  {
    turn -= 0.75f;
  }

  if (event->key() == Qt::Key_D)
  {
    strafing += 1.0f;
  }
  if (event->key() == Qt::Key_A)
  {
    strafing -= 1.0f;
  }

  if (event->key() == Qt::Key_Q)
  {
    updown += 1.0f;
  }
  if (event->key() == Qt::Key_E)
  {
    updown -= 1.0f;
  }

  if (event->key() == Qt::Key_2 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx += 1;
  }
  if (event->key() == Qt::Key_8 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx -= 1;
  }

  if (event->key() == Qt::Key_4 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz += 1;
  }
  if (event->key() == Qt::Key_6 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz -= 1;
  }

  if (event->key() == Qt::Key_3 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy += 1;
  }
  if (event->key() == Qt::Key_1 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy -= 1;
  }

  if (event->key() == Qt::Key_7 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr += 1;
  }
  if (event->key() == Qt::Key_9 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr -= 1;
  }

  if (event->key() == Qt::Key_Plus)
  {
    keys += 1;
  }
  if (event->key() == Qt::Key_Minus)
  {
    keys -= 1;
  }
}

void MapView::keyReleaseEvent (QKeyEvent* event)
{
  if (event->key() == Qt::Key_Shift)
    _mod_shift_down = false;

  if (event->key() == Qt::Key_Alt)
    _mod_alt_down = false;

  if (event->key() == Qt::Key_Control)
    _mod_ctrl_down = false;

  if (event->key() == Qt::Key_Space)
    _mod_space_down = false;

  // movement
  if (event->key() == Qt::Key_W)
  {
    key_w = false;
    if (!(leftMouse && rightMouse))
    {
      moving -= 1.0f;
    }
  }
  if (event->key() == Qt::Key_S)
  {
    moving += 1.0f;
  }

  if (event->key() == Qt::Key_Up)
  {
    lookat -= 0.75f;
  }
  if (event->key() == Qt::Key_Down)
  {
    lookat += 0.75f;
  }

  if (event->key() == Qt::Key_Right)
  {
    turn -= 0.75f;
  }
  if (event->key() == Qt::Key_Left)
  {
    turn += 0.75f;
  }

  if (event->key() == Qt::Key_D)
  {
    strafing -= 1.0f;
  }
  if (event->key() == Qt::Key_A)
  {
    strafing += 1.0f;
  }

  if (event->key() == Qt::Key_Q)
  {
    updown -= 1.0f;
  }
  if (event->key() == Qt::Key_E)
  {
    updown += 1.0f;
  }

  if (event->key() == Qt::Key_2 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx -= 1;
  }
  if (event->key() == Qt::Key_8 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx += 1;
  }

  if (event->key() == Qt::Key_4 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz -= 1;
  }
  if (event->key() == Qt::Key_6 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz += 1;
  }

  if (event->key() == Qt::Key_3 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy -= 1;
  }
  if (event->key() == Qt::Key_1 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy += 1;
  }

  if (event->key() == Qt::Key_7 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr -= 1;
  }
  if (event->key() == Qt::Key_9 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr += 1;
  }

  if (event->key() == Qt::Key_Plus)
  {
    keys -= 1;
  }
  if (event->key() == Qt::Key_Minus)
  {
    keys += 1;
  }
}

void MapView::focusOutEvent (QFocusEvent*)
{
  _mod_alt_down = false;
  _mod_ctrl_down = false;
  _mod_shift_down = false;
  _mod_space_down = false;

  key_w = false;

  moving = 0.0f;
  lookat = 0.0f;
  turn = 0.0f;
  strafing = 0.0f;
  updown = 0.0f;

  keyx = 0;
  keyz = 0;
  keyy = 0;
  keyr = 0;
  keys = 0;

  leftMouse = false;
  rightMouse = false;
  MoveObj = false;
  look = false;
}

void MapView::insert_object_at_selection_position (std::string m2_to_add)
{
  if (!_world->HasSelection())
    return;

  math::vector_3d selectionPosition;
  switch (_world->GetCurrentSelection()->which())
  {
  case eEntry_Model:
    selectionPosition = boost::get<selected_model_type> (*_world->GetCurrentSelection())->pos;
    break;
  case eEntry_WMO:
    selectionPosition = boost::get<selected_wmo_type> (*_world->GetCurrentSelection())->pos;
    break;
  case eEntry_MapChunk:
    selectionPosition = boost::get<selected_chunk_type> (*_world->GetCurrentSelection()).position;
    break;
  }

  if (!MPQFile::exists(m2_to_add))
  {
    LogError << "Failed adding " << m2_to_add << ". It was not in any MPQ." << std::endl;
  }

  _world->addM2(m2_to_add, selectionPosition, false);
  //! \todo Memoryleak: These models will never get deleted.
}

void MapView::mouseMoveEvent (QMouseEvent* event)
{
  //! \todo:  move the function call requiring a context in tick ?
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());
  QLineF const relative_movement (_last_mouse_pos, event->pos());

  if (look && !(_mod_shift_down || _mod_ctrl_down || _mod_alt_down || _mod_space_down))
  {
    _camera.add_to_yaw(math::degrees(relative_movement.dx() / XSENS));
    _camera.add_to_pitch(math::degrees(mousedir * relative_movement.dy() / YSENS));
    _minimap->update();
  }

  if (MoveObj)
  {
    mh = -aspect_ratio()*relative_movement.dx() / static_cast<float>(width());
    mv = -relative_movement.dy() / static_cast<float>(height());
  }
  else
  {
    mh = 0.0f;
    mv = 0.0f;
  }

  if (_mod_shift_down || _mod_ctrl_down || _mod_alt_down || _mod_space_down)
  {
    rh = relative_movement.dx() / XSENS * 5.0f;
    rv = relative_movement.dy() / YSENS * 5.0f;
  }

  if (rightMouse && _mod_alt_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      terrainTool->changeInnerRadius(relative_movement.dx() / 100.0f);
    }
    if (terrainMode == editing_mode::paint)
    {
      texturingTool->change_hardness(relative_movement.dx() / 300.0f);
    }
  }

  if (rightMouse && _mod_shift_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      terrainTool->moveVertices(-relative_movement.dy() / YSENS);
    }
  }


  if (rightMouse && _mod_space_down)
  {
    terrainTool->setOrientRelativeTo(_cursor_pos);
  }

  if (leftMouse && _mod_alt_down)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      terrainTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::flatten_blur:
      flattenTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::paint:
      texturingTool->change_radius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::water:
      guiWater->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::mccv:
      shaderTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    }
  }

  if (leftMouse && _mod_space_down)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      terrainTool->changeSpeed(relative_movement.dx() / 30.0f);
      break;
    case editing_mode::flatten_blur:
      flattenTool->changeSpeed(relative_movement.dx() / 30.0f);
      break;
    case editing_mode::paint:
      texturingTool->change_pressure(relative_movement.dx() / 300.0f);
      break;
    case editing_mode::mccv:
      shaderTool->changeSpeed(relative_movement.dx() / XSENS);
      break;
    }
  }

  if (mViewMode == eViewMode_2D && leftMouse && _mod_alt_down && _mod_shift_down)
  {
    strafing = ((relative_movement.dx() / XSENS) / -1) * 5.0f;
    moving = (relative_movement.dy() / YSENS) * 5.0f;
  }

  if (mViewMode == eViewMode_2D && rightMouse && _mod_shift_down)
  {
    updown = (relative_movement.dy() / YSENS);
  }

  _last_mouse_pos = event->pos();

  checkWaterSave(); // ????? \todo Move to somewhere more appropriate.
}

void MapView::selectModel(selection_type entry)
{
  objectEditor->copy(entry);
}

void MapView::mousePressEvent (QMouseEvent* event)
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  switch (event->button())
  {
  case Qt::LeftButton:
    leftMouse = true;
    break;

  case Qt::RightButton:
    rightMouse = true;
    break;

  case Qt::MiddleButton:
    if (_world->HasSelection())
    {
      MoveObj = true;
      auto selection = _world->GetCurrentSelection();
      math::vector_3d objPos;
      if (selection->which() == eEntry_WMO)
      {
        objPos = boost::get<selected_wmo_type> (*selection)->pos;
      }
      else if (selection->which() == eEntry_Model)
      {
        objPos = boost::get<selected_model_type> (*selection)->pos;
      }

      objMoveOffset = _cursor_pos - objPos;
    }

    break;
  }

  if (leftMouse && rightMouse)
  {
    // Both buttons
    moving = 1.0f;
  }
  else if (leftMouse)
  {
    if (mViewMode == eViewMode_3D)
    {
      doSelection(false);
    }
  }
  else if (rightMouse)
  {
    look = true;
  }
}

void MapView::wheelEvent (QWheelEvent* event)
{
  //! \todo: move the function call requiring a context in tick ?
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  auto&& delta_for_range
    ( [&] (float range)
      {
        //! \note / 8.f for degrees, / 40.f for smoothness
        return (_mod_ctrl_down ? 0.1f : 1.f) * event->angleDelta().y() / 320.f;
      }
    );

  if (terrainMode == editing_mode::ground)
  {
    if (_mod_alt_down)
    {
      terrainTool->changeAngle (delta_for_range (178.f));
    }
    else if (_mod_shift_down)
    {
      terrainTool->changeOrientation (delta_for_range (360.f));
    }
  }
  else if (terrainMode == editing_mode::paint)
  {
    if (_mod_space_down)
    {
      texturingTool->change_brush_level (delta_for_range (255.f));
    }
    else if (_mod_alt_down)
    {
      texturingTool->change_spray_size (delta_for_range (39.f));
    }
    else if (_mod_shift_down)
    {
      texturingTool->change_spray_pressure (delta_for_range (10.f));
    }
  }
  else if (terrainMode == editing_mode::flatten_blur)
  {
    if (_mod_alt_down)
    {
      flattenTool->changeOrientation (delta_for_range (360.f));
    }
    else if (_mod_shift_down)
    {
      flattenTool->changeAngle (delta_for_range (89.f));
    }
    else if (_mod_space_down)
    {
      //! \note not actual range
      flattenTool->changeHeight (delta_for_range (40.f));
    }
  }
  else if (terrainMode == editing_mode::water)
  {
    if (_mod_alt_down)
    {
      guiWater->changeOrientation (delta_for_range (360.f));
    }
    else if (_mod_shift_down)
    {
      guiWater->changeAngle (delta_for_range (89.f));
    }
    else if (_mod_space_down)
    {
      //! \note not actual range
      guiWater->change_height (delta_for_range (40.f));
    }
  }
}

void MapView::mouseReleaseEvent (QMouseEvent* event)
{
  switch (event->button())
  {
  case Qt::LeftButton:
    leftMouse = false;

    if (!key_w && moving > 0.0f)
      moving = 0.0f;

    if (mViewMode == eViewMode_2D)
    {
      strafing = 0;
      moving = 0;
    }
    break;

  case Qt::RightButton:
    rightMouse = false;

    look = false;

    if (!key_w && moving > 0.0f)
      moving = 0.0f;

    if (mViewMode == eViewMode_2D)
      updown = 0;

    break;

  case Qt::MiddleButton:
    MoveObj = false;
    break;
  }
}

void MapView::checkWaterSave()
{
  tile_index const current (_camera.position);

  if (!_world->mapIndex.hasTile (current) || _world->canWaterSave(current))
  {
    waterSaveWarning->hide();
  }
  else
  {
    waterSaveWarning->show();
  }
}

void MapView::prompt_save_current()
{
  if ( QMessageBox::warning
         ( nullptr
         , "Save (only) current map tile"
         , "This can cause a collision bug when placing objects between two ADT borders!\n\n"
           "If you often use this function, we recommend you to use the 'Save all' "
           "function as often as possible to get the collisions right."
         , QMessageBox::Save | QMessageBox::Cancel
         , QMessageBox::Cancel
         ) == QMessageBox::Save
     )
  {
    makeCurrent();
    opengl::context::scoped_setter const _ (::gl, context());
    _world->mapIndex.saveTile(tile_index(_camera.position));
  }
}

void MapView::addHotkey(Qt::Key key, size_t modifiers, std::function<void()> function, std::function<bool()> condition)
{
  hotkeys.emplace_front (key, modifiers, function, condition);
}

#ifdef _WIN32
HCTX static NEAR TabletInit(HWND hWnd)
{
  HCTX hctx = nullptr;
  UINT wDevice = 0;
  UINT wExtX = 0;
  UINT wExtY = 0;
  UINT wWTInfoRetVal = 0;
  AXIS TabletX = { 0 };
  AXIS TabletY = { 0 };

  // Set option to move system cursor before getting default system context.
  glogContext.lcOptions |= CXO_SYSTEM;

  // Open default system context so that we can get tablet data
  // in screen coordinates (not tablet coordinates).
  wWTInfoRetVal = gpWTInfoA(WTI_DEFSYSCTX, 0, &glogContext);
  WACOM_ASSERT(wWTInfoRetVal == sizeof(LOGCONTEXT));

  WACOM_ASSERT(glogContext.lcOptions & CXO_SYSTEM);

  // modify the digitizing region
  wsprintf(glogContext.lcName, "PrsTest Digitizing %x", hInst);

  // We process WT_PACKET (CXO_MESSAGES) messages.
  glogContext.lcOptions |= CXO_MESSAGES;

  // What data items we want to be included in the tablet packets
  glogContext.lcPktData = PACKETDATA;

  // Which packet items should show change in value since the last
  // packet (referred to as 'relative' data) and which items
  // should be 'absolute'.
  glogContext.lcPktMode = PACKETMODE;

  // This bitfield determines whether or not this context will receive
  // a packet when a value for each packet field changes.  This is not
  // supported by the Intuos Wintab.  Your context will always receive
  // packets, even if there has been no change in the data.
  glogContext.lcMoveMask = PACKETDATA;

  // Which buttons events will be handled by this context.  lcBtnMask
  // is a bitfield with one bit per button.
  glogContext.lcBtnUpMask = glogContext.lcBtnDnMask;

  // Set the entire tablet as active
  wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + 0, DVC_X, &TabletX);
  WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

  wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_Y, &TabletY);
  WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

  glogContext.lcInOrgX = 0;
  glogContext.lcInOrgY = 0;
  glogContext.lcInExtX = TabletX.axMax;
  glogContext.lcInExtY = TabletY.axMax;

  // Guarantee the output coordinate space to be in screen coordinates.
  glogContext.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
  glogContext.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
  glogContext.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN); //SM_CXSCREEN );

                                  // In Wintab, the tablet origin is lower left.  Move origin to upper left
                                  // so that it coincides with screen origin.
  glogContext.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);  //SM_CYSCREEN );

                                  // Leave the system origin and extents as received:
                                  // lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

                                  // open the region
                                  // The Wintab spec says we must open the context disabled if we are
                                  // using cursor masks.
  hctx = gpWTOpenA(hWnd, &glogContext, FALSE);

  WacomTrace("HCTX: %i\n", hctx);

  return hctx;
}
#endif

void MapView::init_tablet()
{
#ifdef _WIN32
  //this is for graphics tablet (e.g. Wacom, Huion, possibly others) initialization.
  HWND WindowHandle = (HWND)effectiveWinId();
  hInst = (HINSTANCE)GetWindowLongPtr(WindowHandle, GWLP_HINSTANCE);

  if (LoadWintab())
  {
    /* check if WinTab available. */
    if (gpWTInfoA(0, 0, nullptr))
    {
      hCtx = TabletInit(WindowHandle);
      gpWTEnable(hCtx, TRUE);
      gpWTOverlap(hCtx, TRUE);
      if (!hCtx)
      {
        Log << "Could Not Open Tablet Context." << std::endl;
      }
      else
      {
        Log << "Opened Tablet Context." << std::endl;
      }
      _tablet_active = true;
    }
  }
#endif
}
