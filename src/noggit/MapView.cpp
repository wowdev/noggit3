// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#undef _UNICODE

#include <noggit/Brush.h> // brush
#include <noggit/ConfigFile.h>
#include <noggit/DBC.h>
#include <noggit/Environment.h>
#include <noggit/Environment.h>
#include <noggit/FreeType.h> // freetype::
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
#include <noggit/ui/CheckBox.h> // UICheckBox
#include <noggit/ui/CursorSwitcher.h> // UICursorSwitcher
#include <noggit/ui/DetailInfos.h> // detailInfos
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/MapViewGUI.h> // UIMapViewGUI
#include <noggit/ui/MenuBar.h> // UIMenuBar, menu items, ..
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/Slider.h> // UISlider
#include <noggit/ui/Text.h> // UIText
#include <noggit/ui/Texture.h> // textureUI
#include <noggit/ui/TexturePicker.h>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/ToggleGroup.h> // UIToggleGroup
#include <noggit/ui/Toolbar.h> // noggit::ui::toolbar
#include <noggit/ui/Water.h>
#include <noggit/ui/WaterSaveWarning.h>
#include <noggit/ui/WaterTypeBrowser.h>
#include <noggit/ui/ZoneIDBrowser.h>
#include <noggit/ui/main_window.hpp>
#include <noggit/ui/shader_tool.hpp>
#include <noggit/ui/terrain_tool.hpp>
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

  mainGui->guiWaterTypeSelector->hide();
  mainGui->_terrain->hide();
  mainGui->flattenTool->hide();
  mainGui->texturingTool->hide();
  mainGui->shaderTool->hide();
  mainGui->guiWater->hide();
  mainGui->TexturePicker->hide();
  mainGui->objectEditor->hide();
  mainGui->objectEditor->modelImport->hide();
  mainGui->rotationEditor->hide();
  mainGui->ZoneIDBrowser->hide();

  if (!mainGui || !mainGui->TexturePalette)
    return;
  mainGui->TexturePalette->hide();
  // fetch old win position
  switch (terrainMode)
  {
  case editing_mode::ground:
    break;
  case editing_mode::flatten_blur:
    tool_settings_x = mainGui->flattenTool->x();
    tool_settings_y = mainGui->flattenTool->y();
    break;
  case editing_mode::paint:
    tool_settings_x = mainGui->texturingTool->x();
    tool_settings_y = mainGui->texturingTool->y();
    break;
  case editing_mode::areaid:
    tool_settings_x = mainGui->ZoneIDBrowser->x() + 230;
    tool_settings_y = mainGui->ZoneIDBrowser->y();
    break;
  case editing_mode::water:
    tool_settings_x = mainGui->guiWater->x();
    tool_settings_y = mainGui->guiWater->y();
    break;
  case editing_mode::mccv:
    tool_settings_x = mainGui->shaderTool->x();
    tool_settings_y = mainGui->shaderTool->y();
    break;
  }
  // set new win pos and make visible
  switch (mode)
  {
  case editing_mode::ground:
    mainGui->_terrain->show();
    break;
  case editing_mode::flatten_blur:
    mainGui->flattenTool->move(tool_settings_x, tool_settings_y);
    mainGui->flattenTool->show();
    break;
  case editing_mode::paint:
    mainGui->texturingTool->show();
    break;
  case editing_mode::areaid:
    mainGui->ZoneIDBrowser->move(mainGui->ZoneIDBrowser->x(), tool_settings_y);
    mainGui->ZoneIDBrowser->show();
    break;
  case editing_mode::water:
    mainGui->guiWater->x(tool_settings_x);
    mainGui->guiWater->y(tool_settings_y);
    mainGui->guiWater->show();
    break;
  case editing_mode::mccv:
    mainGui->shaderTool->show();
    break;
  case editing_mode::object:
    mainGui->objectEditor->show();
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
  //! \todo Beautify.

  // Test if there is an selection
  if (!_world->HasSelection())
    return;

  std::string importFile (Settings::getInstance()->wmvLogFile);

  std::string lastModel;

  size_t foundString;
  std::string line;
  std::string findThis;
  std::ifstream fileReader(importFile.c_str());
  if (fileReader.is_open())
  {
    while (!fileReader.eof())
    {
      getline(fileReader, line);
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);

      if (line.find(".m2") != std::string::npos || line.find(".mdx") != std::string::npos)
      {
        // M2 inside line
        // is it the modelviewer log then cut the log messages out
        findThis = "loading model: ";
        foundString = line.find(findThis);
        if (foundString != std::string::npos)
        {
          // cut path
          line = line.substr(foundString + findThis.size());
        }
        else
        {
          // invalid line
          continue;
        }
      }
      // swap mdx to m2
      size_t found = line.rfind(".mdx");
      if (found != std::string::npos)
      {
        line.replace(found, 4, ".m2");
      }

      lastModel = line.substr(0, line.find(".m2") + 3);
    }
  }
  else
  {
    // file not exist, no rights ore other error
    LogError << importFile << std::endl;
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
  //! \todo Beautify.

  if (!_world->HasSelection())
    return;

  std::string importFile (Settings::getInstance()->wmvLogFile);

  std::string lastWMO;

  size_t foundString;
  std::string line;
  std::string findThis;
  std::ifstream fileReader(importFile.c_str());
  if (fileReader.is_open())
  {
    while (!fileReader.eof())
    {
      getline(fileReader, line);
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);

      if (line.find(".wmo") != std::string::npos)
      {
        // WMO inside line
        findThis = "loading wmo ";
        foundString = line.find(findThis);
        // is it the modelviewer log then cut the log messages out
        if (foundString != std::string::npos)
        {
          // cut path
          line = line.substr(foundString + findThis.size());
        }
        else
        {
          // invalid line
          continue;
        }

        lastWMO = line.substr(0, line.find(".wmo") + 4);
      }
    }
  }
  else
  {
    // file not exist, no rights ore other error
    LogError << importFile << std::endl;
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
  // create main gui object that holds all other gui elements for access ( in the future ;) )
  mainGui = new UIMapViewGUI(this, &_camera, &_tablet_pressure, &_tablet_active);

  mainGui->ZoneIDBrowser->setMapID(_world->getMapID());
  mainGui->ZoneIDBrowser->setChangeFunc([this] (int id){ changeZoneIDValue (id); });
  tool_settings_x = width() - 186;
  tool_settings_y = 38;

  mainGui->terrainTool->storeCursorPos (&_cursor_pos);

  mainGui->addChild(MapChunkWindow = UITexturingGUI::createMapChunkWindow());
  MapChunkWindow->hide();

  _toolbar = new noggit::ui::toolbar([this] (editing_mode mode) { set_editing_mode (mode); });
  _main_window->addToolBar(Qt::LeftToolBarArea, _toolbar);
  connect (this, &QObject::destroyed, _toolbar, &QObject::deleteLater);

  // create the menu
  UIMenuBar * mbar = new UIMenuBar();

  auto file_menu (_main_window->menuBar()->addMenu ("Editor"));
  connect (this, &QObject::destroyed, file_menu, &QObject::deleteLater);

  auto edit_menu (_main_window->menuBar()->addMenu ("Edit"));
  connect (this, &QObject::destroyed, edit_menu, &QObject::deleteLater);

  auto assist_menu (_main_window->menuBar()->addMenu ("Assist"));
  connect (this, &QObject::destroyed, assist_menu, &QObject::deleteLater);

  auto view_menu (_main_window->menuBar()->addMenu ("View"));
  connect (this, &QObject::destroyed, view_menu, &QObject::deleteLater);

  mbar->AddMenu("View");
  mbar->AddMenu("Help");

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
            , &property_, &bool_toggle_property::set              \
            );                                                    \
    connect ( &property_, &bool_toggle_property::changed          \
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
            , &property_, &bool_toggle_property::set              \
            );                                                    \
    connect ( &property_, &bool_toggle_property::changed          \
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
  ADD_ACTION_NS (assist_menu, "Helper models", [this] { mainGui->HelperModels->show(); });

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

  mbar->GetMenu("View")->AddMenuItemSeperator("Windows");

  mbar->GetMenu("View")->AddMenuItemToggle("Texture palette", mainGui->TexturePalette->hidden_evil(), true);
  addHotkey ( Qt::Key_X
            , MOD_none
            , [this] { mainGui->TexturePalette->toggleVisibility(); }
            , [this] { return terrainMode == editing_mode::paint; }
            );

  mbar->GetMenu("View")->AddMenuItemButton("Cursor options", [this] { mainGui->showCursorSwitcher(); });
  addHotkey ( Qt::Key_C
            , MOD_alt | MOD_ctrl
            , [this]
              {
                mainGui->toggleCursorSwitcher();
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

  view_menu->addSection ("Windows");
  ADD_TOGGLE (view_menu, "Detail infos", Qt::Key_F8, _show_detail_info_window);
  connect ( &_show_detail_info_window, &bool_toggle_property::changed
          , mainGui->guidetailInfos, &QWidget::setVisible
          );
  ADD_TOGGLE (view_menu, "Minimap", Qt::Key_M, _show_minimap_window);
  connect ( &_show_minimap_window, &bool_toggle_property::changed
          , _minimap, &QWidget::setVisible
          );

  mbar->GetMenu("View")->AddMenuItemSeperator("Toggle");
  mbar->GetMenu("View")->AddMenuItemToggle("F12 Fog", &_world->drawfog);
  addHotkey(Qt::Key_F12, MOD_none, [this] { _world->drawfog = !_world->drawfog; });

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
                  alloff_fog = _world->drawfog;
                  alloff_terrain = _draw_terrain.get();

                  _draw_models.set (false);
                  _draw_wmo_doodads.set (false);
                  _draw_contour.set (true);
                  _draw_wmo.set (false);
                  _draw_terrain.set (true);
                  _world->drawfog = false;
                }
                else
                {
                  _draw_models.set (alloff_models);
                  _draw_wmo_doodads.set (alloff_doodads);
                  _draw_contour.set (alloff_contour);
                  _draw_wmo.set (alloff_wmo);
                  _draw_terrain.set (alloff_terrain);
                  _world->drawfog = alloff_fog;
                }
                alloff = !alloff;
              }
            );


  mbar->GetMenu("Help")->AddMenuItemButton("H Key Bindings", [this] { mainGui->showHelp(); });
  addHotkey ( Qt::Key_H
            , MOD_none
            , [&]
              {
                mainGui->toggleHelp();
              }
            , [&] { return terrainMode != editing_mode::object; }
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

  mainGui->addChild(mbar);

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

  ADD_ACTION (view_menu, "Show GUI", Qt::Key_Tab, [this] { _GUIDisplayingEnabled = !_GUIDisplayingEnabled; });

  addHotkey ( Qt::Key_C
            , MOD_ctrl
            , [this]
              {
                mainGui->objectEditor->copy (*_world->GetCurrentSelection());
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
                mainGui->objectEditor->copy(*_world->GetCurrentSelection());
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

  addHotkey (Qt::Key_V, MOD_ctrl, [this] { mainGui->objectEditor->pasteObject (_cursor_pos, _camera.position); });
  addHotkey ( Qt::Key_V
            , MOD_none
            , [this] { mainGui->objectEditor->pasteObject (_cursor_pos, _camera.position); }
            , [this] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_C
            , MOD_none
            , [this] { _world->clearVertexSelection(); }
            , [this] { return terrainMode == editing_mode::ground; }
            );

  ADD_ACTION (view_menu, "toggle detail infos", "Ctrl+X", [this] { mainGui->guidetailInfos->toggle_visibility(); });

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
            , [this] { mainGui->terrainTool->nextType(); }
            , [this] { return terrainMode == editing_mode::ground; }
            );

  addHotkey ( Qt::Key_Y
            , MOD_none
            , [this] { mainGui->flattenTool->nextFlattenType(); }
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
                //! \todo space as global modifier?
                if (_mod_space_down)
                {
                  mainGui->flattenTool->nextFlattenMode();
                }
                else
                {
                  mainGui->flattenTool->toggleFlattenAngle();
                }
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );

  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                mainGui->texturingTool->toggle_spray();
              }
            , [&] { return terrainMode == editing_mode::paint; }
            );

  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                _world->setHoleADT (_camera.position, _mod_alt_down);
              }
            , [&] { return terrainMode == editing_mode::holes; }
            );

  addHotkey( Qt::Key_T
           , MOD_none
           , [&]
             {
               mainGui->guiWater->toggle_angled_mode();
             }
          , [&] { return terrainMode == editing_mode::water; }
          );


  addHotkey ( Qt::Key_T
            , MOD_none
            , [&]
              {
                mainGui->objectEditor->togglePasteMode();
              }
            , [&] { return terrainMode == editing_mode::object; }
            );


  addHotkey ( Qt::Key_H
            , MOD_none
            , [&]
              {
                // toggle hidden models visibility
                if (_mod_space_down)
                {
                  _draw_hidden_models.set (!_draw_hidden_models.get());
                }
                else if (_mod_shift_down)
                {
                  _hidden_map_objects.clear();
                  _hidden_models.clear();
                }
                else
                {
                  // toggle selected model visibility
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
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_F
            , MOD_none
            , [&]
              {
                if (_mod_space_down)
                {
                  mainGui->terrainTool->flattenVertices();
                }
              }
            , [&] { return terrainMode == editing_mode::ground; }
            );
  addHotkey ( Qt::Key_F
            , MOD_none
            , [&]
              {
                if (_mod_space_down)
                {
                  mainGui->flattenTool->toggleFlattenLock();
                }
                else
                {
                  mainGui->flattenTool->lockPos (_cursor_pos);
                }
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );
  addHotkey( Qt::Key_F
            , MOD_none
            , [&]
              {
                if (_mod_space_down)
                {
                  mainGui->guiWater->toggle_lock();
                }
                else
                {
                  mainGui->guiWater->lockPos(_cursor_pos);
                }
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

  addHotkey (Qt::Key_Plus, MOD_alt, [this] { mainGui->terrainTool->changeRadius(0.01f); }, [this] { return terrainMode == editing_mode::ground; });

  addHotkey (Qt::Key_Plus, MOD_alt, [this] { mainGui->flattenTool->changeRadius(0.01f); }, [this] { return terrainMode == editing_mode::flatten_blur; });

  addHotkey ( Qt::Key_Plus
            , MOD_alt
            , [&]
              {
                mainGui->texturingTool->change_radius(0.1f);
              }
            , [this] { return terrainMode == editing_mode::paint; }
            );

  addHotkey (Qt::Key_Plus, MOD_shift, [this] { _world->fogdistance += 60.0f; });


  addHotkey (Qt::Key_Minus, MOD_alt, [this] { mainGui->terrainTool->changeRadius(-0.01f); }, [this] { return terrainMode == editing_mode::ground; });

  addHotkey (Qt::Key_Minus, MOD_alt, [this] { mainGui->flattenTool->changeRadius(-0.01f); }, [this] { return terrainMode == editing_mode::flatten_blur; });

  addHotkey ( Qt::Key_Minus
            , MOD_alt
            , [&]
              {
                mainGui->texturingTool->change_radius(-0.1f);
              }
            , [this] { return terrainMode == editing_mode::paint; }
            );

  addHotkey (Qt::Key_Minus, MOD_shift, [this] { _world->fogdistance -= 60.0f; });

  addHotkey (Qt::Key_1, MOD_shift, [this] { _camera.move_speed = 15.0f; });
  addHotkey (Qt::Key_2, MOD_shift, [this] { _camera.move_speed = 50.0f; });
  addHotkey (Qt::Key_3, MOD_shift, [this] { _camera.move_speed = 200.0f; });
  addHotkey (Qt::Key_4, MOD_shift, [this] { _camera.move_speed = 800.0f; });
  addHotkey (Qt::Key_1, MOD_alt, [this] { mainGui->texturingTool->set_brush_level(0.0f); });
  addHotkey (Qt::Key_2, MOD_alt, [this] { mainGui->texturingTool->set_brush_level(255.0f* 0.25f); });
  addHotkey (Qt::Key_3, MOD_alt, [this] { mainGui->texturingTool->set_brush_level(255.0f* 0.5f); });
  addHotkey (Qt::Key_4, MOD_alt, [this] { mainGui->texturingTool->set_brush_level(255.0f* 0.75f); });
  addHotkey (Qt::Key_5, MOD_alt, [this] { mainGui->texturingTool->set_brush_level(255.0f); });

  addHotkey (Qt::Key_1, MOD_none, [this] { set_editing_mode (editing_mode::ground); });
  addHotkey (Qt::Key_2, MOD_none, [this] { set_editing_mode (editing_mode::flatten_blur); });
  addHotkey (Qt::Key_3, MOD_none, [this] { set_editing_mode (editing_mode::paint); });
  addHotkey (Qt::Key_4, MOD_none, [this] { set_editing_mode (editing_mode::holes); });
  addHotkey (Qt::Key_5, MOD_none, [this] { set_editing_mode (editing_mode::areaid); });
  addHotkey (Qt::Key_6, MOD_none, [this] { set_editing_mode (editing_mode::flags); });
  addHotkey (Qt::Key_7, MOD_none, [this] { set_editing_mode (editing_mode::water); });
  addHotkey (Qt::Key_8, MOD_none, [this] { set_editing_mode (editing_mode::light); });
  addHotkey (Qt::Key_9, MOD_none, [this] { set_editing_mode (editing_mode::mccv); });
  addHotkey (Qt::Key_0, MOD_none, [this] { set_editing_mode (editing_mode::object); });

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
  mainGui->waterSaveWarning = new ui::water_save_warning;
  mainGui->waterSaveWarning->hide();

  // modelimport
  mainGui->objectEditor->modelImport = new UIModelImport(this);

  // helper models
  mainGui->HelperModels = new UIHelperModels(this);
}

MapView::MapView( math::degrees camera_yaw0
                , math::degrees camera_pitch0
                , math::vector_3d camera_pos
                , noggit::ui::main_window* main_window
                , World* world
                )
  : _GUIDisplayingEnabled(true)
  , _camera (camera_pos, camera_yaw0, camera_pitch0)
  , mTimespeed(0.0f)
  , _main_window (main_window)
  , _world (world)
  , _status_position (new QLabel (this))
  , _status_selection (new QLabel (this))
  , _status_area (new QLabel (this))
  , _status_time (new QLabel (this))
  , _minimap (new noggit::ui::minimap_widget (nullptr))
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


  setWindowTitle ("Noggit Studio - " STRPRODUCTVER);

  LastClicked = nullptr;

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

  connect (this, &QObject::destroyed
          , []
            {
              TextureManager::report();
              ModelManager::report();
              WMOManager::report();

              app.asyncLoader->stop();
              app.asyncLoader->join();

              MPQArchive::unloadAllMPQs();
              gListfile.clear();

              LogDebug << "Exited" << std::endl;

              QApplication::quit();
            }
          );

  init_tablet();

  _startup_time.start();
  _update_every_event_loop.start (0);
  connect (&_update_every_event_loop, &QTimer::timeout, [this] { update(); });
}

  void MapView::initializeGL()
  {
    opengl::context::scoped_setter const _ (::gl, context());
    gl.viewport(0.0f, 0.0f, width(), height());

    gl.clearColor (0.0f, 0.0f, 0.0f, 1.0f);
    video::width = width();
    video::height = height();

    app.initFont();
    createGUI();

  set_editing_mode (editing_mode::ground);


  // Set camera y (height) position to current ground height plus some space.
  math::vector_3d t = math::vector_3d(0, 0, 0);
  tile_index tile(_camera.position);
  if (!_world->mapIndex.tileLoaded(tile))
  {
    _world->mapIndex.loadTile(tile);
  }

  _world->GetVertex(_camera.position.x, _camera.position.z, &t);

  // min elevation according to https://wowdev.wiki/AreaTable.dbc
  //! \ todo use the current area's MinElevation
  if (t.y < -5000.0f)
  {
    //! \todo use the height of a model/wmo of the tile (or the map) ?
    t.y = 0.0f;
  }

  _camera.position.y = t.y + 50.0f;

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
      display();
    }
  }

  void MapView::resizeGL (int width, int height)
  {
    opengl::context::scoped_setter const _ (::gl, context());
    gl.viewport(0.0f, 0.0f, width, height);

    video::width = width;
    video::height = height;
    mainGui->resize();
  }


MapView::~MapView()
{
  delete mainGui;
  mainGui = nullptr;
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
      mainGui->terrainTool->setRadius (_tablet_pressure / 20.0f);
    case editing_mode::flatten_blur:
      mainGui->flattenTool->setRadius (_tablet_pressure / 20.0f);
      break;
    case editing_mode::paint:
      mainGui->texturingTool->change_pressure (_tablet_pressure / 2048.0f);
      break;
    }
  }
#endif

  if (hasFocus())
  {
    math::vector_3d dir(1.0f, 0.0f, 0.0f);
    math::vector_3d dirUp(1.0f, 0.0f, 0.0f);
    math::vector_3d dirRight(0.0f, 0.0f, 1.0f);
    math::rotate(0.0f, 0.0f, &dir.x, &dir.y, _camera.pitch());
    math::rotate(0.0f, 0.0f, &dir.x, &dir.z, _camera.yaw());

    if (_mod_shift_down)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      dirRight *= 0.0f; //! \todo  WAT?
    }
    else if (_mod_ctrl_down)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.y, _camera.pitch());
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.y, _camera.pitch());
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, _camera.yaw());
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, _camera.yaw());
    }
    else
    {
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, _camera.yaw());
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, _camera.yaw());
    }
    auto Selection = _world->GetCurrentSelection();
    if (Selection)
    {
      // update rotation editor if the selection has changed
      if (!lastSelected || lastSelected != Selection)
      {
        mainGui->rotationEditor->select(*Selection);
      }

      bool canMoveObj = !mainGui->rotationEditor->hasFocus();

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
          mainGui->rotationEditor->updateValues();
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
          mainGui->rotationEditor->updateValues();
        }
      }

      math::vector_3d ObjPos;
      if (_world->IsSelection(eEntry_Model))
      {
        //! \todo  Tell me what this is.
        ObjPos = boost::get<selected_model_type> (*Selection)->pos - _camera.position;
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.y, math::degrees(_camera.pitch()));
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.z, math::degrees(_camera.yaw()));
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
            boost::get<selected_wmo_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
            boost::get<selected_wmo_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
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
          mainGui->rotationEditor->updateValues();
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
              boost::get<selected_model_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
              boost::get<selected_model_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
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

          mainGui->rotationEditor->updateValues();
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

          mainGui->rotationEditor->updateValues();

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
              mainGui->terrainTool->changeTerrain(_cursor_pos, 7.5f * dt);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->terrainTool->changeTerrain(_cursor_pos, -7.5f * dt);
            }
          }
          break;
        case editing_mode::flatten_blur:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->flattenTool->flatten(_cursor_pos, dt);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->flattenTool->blur(_cursor_pos, dt);
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
            mainGui->TexturePicker->getTextures(*_world->GetCurrentSelection());
          }
          else  if (_mod_shift_down && !!UITexturingGUI::getSelectedTexture())
          {
            if (mViewMode == eViewMode_3D && !underMap)
            {
              mainGui->texturingTool->paint(_cursor_pos, dt, *UITexturingGUI::getSelectedTexture());
            }
            else if (mViewMode == eViewMode_2D)
            {
              math::vector_3d pos( CHUNKSIZE * 4.0f * aspect_ratio() * ((float)_last_mouse_pos.x() / (float)width() - 0.5f ) / _2d_zoom
                                  , 0.0f
                                  , CHUNKSIZE * 4.0f * ((float)_last_mouse_pos.y() / (float)height() - 0.5f) / _2d_zoom
                                  );

              pos += _camera.position;
              mainGui->texturingTool->paint(pos, dt, *UITexturingGUI::getSelectedTexture());
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
              mainGui->ZoneIDBrowser->setZoneID(newID);
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
              mainGui->guiWater->paintLiquid(_cursor_pos, true);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->guiWater->paintLiquid(_cursor_pos, false);
            }
          }
          break;
        case editing_mode::mccv:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->shaderTool->changeShader(_cursor_pos, dt, true);
            }
            if (_mod_ctrl_down)
            {
              mainGui->shaderTool->changeShader(_cursor_pos, dt, false);
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
  }
  else
  {
    leftMouse = false;
    rightMouse = false;
    look = false;
    MoveObj = false;

    moving = 0;
    strafing = 0;
    updown = 0;
  }

  mainGui->texturingTool->update_brushes();


  _world->time += this->mTimespeed * dt;


  _world->animtime += dt * 1000.0f;
  globalTime = static_cast<int>(_world->animtime);

  _world->tick (dt);

  lastSelected = _world->GetCurrentSelection();

  if (!MapChunkWindow->hidden() && _world->GetCurrentSelection() && _world->GetCurrentSelection()->which() == eEntry_MapChunk)
  {
    UITexturingGUI::setChunkWindow(boost::get<selected_chunk_type> (*_world->GetCurrentSelection()).chunk);
  }

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

void MapView::displayGUIIfEnabled()
{
  if (_GUIDisplayingEnabled)
  {
    gl.matrixMode (GL_PROJECTION);
    gl.loadIdentity();
    gl.ortho (0.0f, width(), height(), 0.0f, -1.0f, 1.0f);
    gl.matrixMode (GL_MODELVIEW);
    gl.loadIdentity();

    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    opengl::texture::disable_texture(1);
    opengl::texture::enable_texture(0);

    gl.disable(GL_DEPTH_TEST);
    gl.disable(GL_CULL_FACE);
    gl.disable(GL_LIGHTING);
    gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);

    opengl::texture::disable_texture(0);

    mainGui->setTilemode(mViewMode != eViewMode_3D);
    mainGui->render();

    opengl::texture::enable_texture(0);
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

    mainGui->texturingTool->bind_brush_texture();

    const float tRadius = mainGui->texturingTool->brush_radius() / CHUNKSIZE;// *_2d_zoom;
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

  displayGUIIfEnabled();
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
    radius = mainGui->terrainTool->brushRadius();
    inner_radius = mainGui->terrainTool->innerRadius();
    break;
  case editing_mode::flatten_blur:
    radius = mainGui->flattenTool->brushRadius();
    angle = mainGui->flattenTool->angle();
    orientation = mainGui->flattenTool->orientation();
    ref_pos = mainGui->flattenTool->ref_pos();
    angled_mode = mainGui->flattenTool->angled_mode();
    use_ref_pos = mainGui->flattenTool->use_ref_pos();
    break;
  case editing_mode::paint:
    radius = mainGui->texturingTool->brush_radius();
    hardness = mainGui->texturingTool->hardness();
    break;
  case editing_mode::water:
    radius = mainGui->guiWater->brushRadius();
    angle = mainGui->guiWater->angle();
    orientation = mainGui->guiWater->orientation();
    ref_pos = mainGui->guiWater->ref_pos();
    angled_mode = mainGui->guiWater->angled_mode();
    use_ref_pos = mainGui->guiWater->use_ref_pos();
    break;
  case editing_mode::mccv:
    radius = mainGui->shaderTool->brushRadius();
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
               , mainGui->texturingTool->show_unpaintable_chunks()
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
               );

  displayGUIIfEnabled();
}

void MapView::display()
{
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

void MapView::keyPressEvent (QKeyEvent *event)
{
  if (handleHotkeys (event))
    return;

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
    moving = 1.0f;
  }

  if (event->key() == Qt::Key_Up)
  {
    lookat = 0.75f;
  }

  if (event->key() == Qt::Key_Down)
  {
    lookat = -0.75f;
  }

  if (event->key() == Qt::Key_Left)
  {
    turn = -0.75f;
  }

  if (event->key() == Qt::Key_Right)
  {
    turn = 0.75f;
  }

  // save
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

  // position correction with num pad
  //! \todo revive
  /*
  if (event->key() == SDLK_KP8)
    keyx = -1;

  if (event->key() == SDLK_KP2)
    keyx = 1;

  if (event->key() == SDLK_KP6)
    keyz = -1;

  if (event->key() == SDLK_KP4)
    keyz = 1;

  if (event->key() == SDLK_KP1)
    keyy = -1;

  if (event->key() == SDLK_KP3)
    keyy = 1;

  if (event->key() == SDLK_KP7)
    keyr = 1;

  if (event->key() == SDLK_KP9)
    keyr = -1;
  */

  // fog distance or brush radius
  if (event->key() == Qt::Key_Plus)
  {
    //change selected model size
    if (_world->HasSelection() && _world->GetCurrentSelection()->which() != eEntry_MapChunk)
      keys = 1;
  }

  if (event->key() == Qt::Key_Minus)
  {
    //change selected model size
    if (_world->HasSelection() && _world->GetCurrentSelection()->which() != eEntry_MapChunk)
      keys = -1;
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
    if (!(leftMouse && rightMouse) && moving > 0.0f)
    {
      moving = 0.0f;
    }
  }

  if (event->key() == Qt::Key_S && moving < 0.0f)
  {
    moving = 0.0f;
  }

  if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
    lookat = 0.0f;

  if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
    turn = 0.0f;

  if (event->key() == Qt::Key_D && strafing > 0.0f)
  {
    strafing = 0.0f;
  }

  if (event->key() == Qt::Key_A && strafing < 0.0f)
  {
    strafing = 0.0f;
  }

  if (event->key() == Qt::Key_Q && updown > 0.0f)
    updown = 0.0f;

  if (event->key() == Qt::Key_E && updown < 0.0f)
    updown = 0.0f;

  //! \todo revive
  /*
  if (event->key() == SDLK_KP8)
    keyx = 0;

  if (event->key() == SDLK_KP2)
    keyx = 0;

  if (event->key() == SDLK_KP6)
    keyz = 0;

  if (event->key() == SDLK_KP4)
    keyz = 0;

  if (event->key() == SDLK_KP1)
    keyy = 0;

  if (event->key() == SDLK_KP3)
    keyy = 0;

  if (event->key() == SDLK_KP7)
    keyr = 0;

  if (event->key() == SDLK_KP9)
    keyr = 0;
  */

  if (event->key() == Qt::Key_Minus || event->key() == Qt::Key_Plus)
    keys = 0;
}

void MapView::inserObjectFromExtern(int model)
{
  if (!_world->HasSelection())
    return;

  std::string m2_to_add;

  const char* filesToAdd[15] = { ""
                                 , ""
                                 , "World\\Scale\\humanmalescale.m2"
                                 , "World\\Scale\\50x50.m2"
                                 , "World\\Scale\\100x100.m2"
                                 , "World\\Scale\\250x250.m2"
                                 , "World\\Scale\\500x500.m2"
                                 , "World\\Scale\\1000x1000.m2"
                                 , "World\\Scale\\50yardradiusdisc.m2"
                                 , "World\\Scale\\200yardradiusdisc.m2"
                                 , "World\\Scale\\777yardradiusdisc.m2"
                                 , "World\\Scale\\50yardradiussphere.m2"
                                 , "World\\Scale\\200yardradiussphere.m2"
                                 , "World\\Scale\\777yardradiussphere.m2"
                                 , ""
  };

  m2_to_add = filesToAdd[model];

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
      mainGui->terrainTool->changeInnerRadius(relative_movement.dx() / 100.0f);
    }
    if (terrainMode == editing_mode::paint)
    {
      mainGui->texturingTool->change_hardness(relative_movement.dx() / 300.0f);
    }
  }

  if (rightMouse && _mod_shift_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      mainGui->terrainTool->moveVertices(-relative_movement.dy() / YSENS);
    }
  }


  if (rightMouse && _mod_space_down)
  {
    mainGui->terrainTool->setOrientRelativeTo(_cursor_pos);
  }

  if (leftMouse && _mod_alt_down)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      mainGui->terrainTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::flatten_blur:
      mainGui->flattenTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::paint:
      mainGui->texturingTool->change_radius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::water:
      mainGui->guiWater->changeRadius(relative_movement.dx() / XSENS);
      break;
    case editing_mode::mccv:
      mainGui->shaderTool->changeRadius(relative_movement.dx() / XSENS);
      break;
    }
  }

  if (leftMouse && _mod_space_down)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      mainGui->terrainTool->changeSpeed(relative_movement.dx() / 30.0f);
      break;
    case editing_mode::flatten_blur:
      mainGui->flattenTool->changeSpeed(relative_movement.dx() / 30.0f);
      break;
    case editing_mode::paint:
      mainGui->texturingTool->change_pressure(relative_movement.dx() / 300.0f);
      break;
    case editing_mode::mccv:
      mainGui->shaderTool->changeSpeed(relative_movement.dx() / XSENS);
      break;
    }
  }

  if (leftMouse && LastClicked)
  {
    LastClicked->processLeftDrag((float)(event->pos().x() - 4), (float)(event->pos().y() - 4), relative_movement.dx(), relative_movement.dy());
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

  mainGui->mouse_moved (event->pos().x(), event->pos().y());

  _last_mouse_pos = event->pos();

  checkWaterSave(); // ????? \todo Move to somewhere more appropriate.
}

void MapView::selectModel(selection_type entry)
{
  mainGui->objectEditor->copy(entry);
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
    LastClicked = mainGui->processLeftClick (event->pos().x(), event->pos().y());
    if (mViewMode == eViewMode_3D && !LastClicked)
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

  //! \todo don't just use distance but delta
  float delta (event->angleDelta().y());

  if (delta > 0.f)
  {
    if (terrainMode == editing_mode::ground)
    {
      if (_mod_alt_down)
      {
        mainGui->terrainTool->changeAngle(_mod_ctrl_down ? 0.2f : 2.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->terrainTool->changeOrientation(_mod_ctrl_down ? 1.0f : 10.0f);
      }
    }
    else if (terrainMode == editing_mode::flatten_blur)
    {
      if (_mod_alt_down)
      {
        mainGui->flattenTool->changeOrientation(_mod_ctrl_down ? 1.0f : 10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->flattenTool->changeAngle(_mod_ctrl_down ? 0.2f : 2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->flattenTool->changeHeight(1.0f);
      }
    }
    else if (terrainMode == editing_mode::paint)
    {
      if (_mod_space_down)
      {
        mainGui->texturingTool->change_brush_level(10.0f);
      }
      else if (_mod_alt_down)
      {
        mainGui->texturingTool->change_spray_size(1.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->texturingTool->change_spray_pressure(0.25);
      }
    }
    else if (terrainMode == editing_mode::water)
    {
      if (_mod_alt_down)
      {
        mainGui->guiWater->changeOrientation(_mod_ctrl_down ? 1.0f : 10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->guiWater->changeAngle(_mod_ctrl_down ? 0.2f : 2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->guiWater->change_height(_mod_ctrl_down ? 0.1f : 1.0f);
      }
    }
  }
  else
  {
    if (terrainMode == editing_mode::ground)
    {
      if (_mod_alt_down)
      {
        mainGui->terrainTool->changeAngle(_mod_ctrl_down ? -0.2f : -2.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->terrainTool->changeOrientation(_mod_ctrl_down ? -1.0f : -10.0f);
      }
    }
    else if (terrainMode == editing_mode::flatten_blur)
    {
      if (_mod_alt_down)
      {
        mainGui->flattenTool->changeOrientation(_mod_ctrl_down ? -1.0f : -10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->flattenTool->changeAngle(_mod_ctrl_down ? -0.2f : -2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->flattenTool->changeHeight(-1.0f);
      }
    }
    else if (terrainMode == editing_mode::paint)
    {
      if (_mod_space_down)
      {
        mainGui->texturingTool->change_brush_level(-10.0f);
      }
      else if (_mod_alt_down)
      {
        mainGui->texturingTool->change_spray_size(-1.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->texturingTool->change_spray_pressure(-0.25);
      }
    }
    else if (terrainMode == editing_mode::water)
    {
      if (_mod_alt_down)
      {
        mainGui->guiWater->changeOrientation(_mod_ctrl_down ? -1.0f : -10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->guiWater->changeAngle(_mod_ctrl_down ? -0.2f : -2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->guiWater->change_height(_mod_ctrl_down ? -0.1f : -1.0f);
      }
    }
  }
}

void MapView::mouseReleaseEvent (QMouseEvent* event)
{
  switch (event->button())
  {
  case Qt::LeftButton:
    leftMouse = false;

    if (LastClicked)
      LastClicked->processUnclick();

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
    mainGui->waterSaveWarning->hide();
  }
  else
  {
    mainGui->waterSaveWarning->show();
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

bool MapView::handleHotkeys(QKeyEvent* event)
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  size_t modifier = (event->modifiers() == Qt::NoModifier) ? (MOD_none) : (
    ((event->modifiers() & Qt::ShiftModifier) ? MOD_shift : 0) |
    ((event->modifiers() & Qt::ControlModifier) ? MOD_ctrl : 0) |
    ((event->modifiers() & Qt::AltModifier) ? MOD_alt : 0) |
    ((event->modifiers() & Qt::MetaModifier) ? MOD_meta : 0));

  for (auto&& hotkey : hotkeys)
  {
    if (event->key() == hotkey.key && modifier == hotkey.modifiers && hotkey.condition())
    {
      hotkey.function();
      return true;
    }
  }
  return false;
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
