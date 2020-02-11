// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/projection.hpp>
#include <noggit/Brush.h> // brush
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/map_index.hpp>
#include <noggit/uid_storage.hpp>
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
#include <noggit/ui/ZoneIDBrowser.h>
#include <noggit/ui/main_window.hpp>
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/shader_tool.hpp>
#include <noggit/ui/terrain_tool.hpp>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/texturing_tool.hpp>
#include <noggit/ui/texture_palette_small.hpp>
#include <opengl/scoped.hpp>

#include "revision.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QWidgetAction>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>


static const float XSENS = 15.0f;
static const float YSENS = 15.0f;

void MapView::set_editing_mode (editing_mode mode)
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  objectEditor->modelImport->hide();
  objectEditor->rotationEditor->hide();
  TexturePalette->hide();
  TexturePicker->hide();
  _texture_palette_dock->hide();

  MoveObj = false;
  _world->reset_selection();
  _rotation_editor_need_update = true;

  if (!ui_hidden)
  {
    setToolPropertyWidgetVisibility(mode);
  }

  terrainMode = mode;
  _toolbar->check_tool (mode);
}

void MapView::setToolPropertyWidgetVisibility(editing_mode mode)
{
  for (auto dock : _tool_properties_docks)
  {
    dock->hide();
  }

  switch (mode)
  {
  case editing_mode::ground:
    _terrain_tool_dock->setVisible(!ui_hidden);
    break;
  case editing_mode::flatten_blur:
    _flatten_blur_dock->setVisible(!ui_hidden);
    break;
  case editing_mode::paint:
    _texturing_dock->setVisible(!ui_hidden);
    break;
  case editing_mode::areaid:
    _areaid_editor_dock->setVisible(!ui_hidden);
    break;
  case editing_mode::water:
    _water_editor_dock->setVisible(!ui_hidden);
    break;
  case editing_mode::mccv:
    _vertex_shading_dock->setVisible(!ui_hidden);
    break;
  case editing_mode::object:
    _object_editor_dock->setVisible(!ui_hidden);
    break;
  }

  
}

void MapView::ResetSelectedObjectRotation()
{
  for (auto& selection : _world->current_selection())
  {
    if (selection.which() == eEntry_WMO)
    {
      WMOInstance* wmo = boost::get<selected_wmo_type>(selection);
      _world->updateTilesWMO(wmo, model_update::remove);
      wmo->resetDirection();
      _world->updateTilesWMO(wmo, model_update::add);
    }
    else if (selection.which() == eEntry_Model)
    {
      ModelInstance* m2 = boost::get<selected_model_type>(selection);
      _world->updateTilesModel(m2, model_update::remove);
      m2->resetDirection();
      m2->recalcExtents();
      _world->updateTilesModel(m2, model_update::add);
    }
  }

  _rotation_editor_need_update = true;
}

void MapView::snap_selected_models_to_the_ground()
{
  _world->snap_selected_models_to_the_ground();
  _rotation_editor_need_update = true;
}


void MapView::DeleteSelectedObject()
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  _world->delete_selected_models();
  _rotation_editor_need_update = true;
}


void MapView::changeZoneIDValue (int set)
{
  _selected_area_id = set;
}


QWidgetAction* MapView::createTextSeparator(const QString& text)
{
  auto* pLabel = new QLabel(text);
  //pLabel->setMinimumWidth(this->minimumWidth() - 4);
  pLabel->setAlignment(Qt::AlignCenter);
  auto* separator = new QWidgetAction(this);
  separator->setDefaultWidget(pLabel);
  return separator;
}


void MapView::createGUI()
{
  // create tool widgets

  _terrain_tool_dock = new QDockWidget("Raise / Lower", this);
  terrainTool = new noggit::ui::terrain_tool(_terrain_tool_dock);
  _terrain_tool_dock->setWidget(terrainTool);
  _tool_properties_docks.insert(_terrain_tool_dock);

  connect(terrainTool
    , &noggit::ui::terrain_tool::updateVertices
    , [this](int vertex_mode, math::degrees const& angle, math::degrees const& orientation)
    {
      makeCurrent();
      opengl::context::scoped_setter const _(::gl, context());

      _world->orientVertices(vertex_mode == eVertexMode_Mouse
        ? _cursor_pos
        : _world->vertexCenter()
        , angle
        , orientation
      );
    }
  );

  _flatten_blur_dock = new QDockWidget("Flatten / Blur", this);
  flattenTool = new noggit::ui::flatten_blur_tool(_flatten_blur_dock);
  _flatten_blur_dock->setWidget(flattenTool);
  _tool_properties_docks.insert(_flatten_blur_dock);

  _texturing_dock = new QDockWidget("Texturing", this);
  texturingTool = new noggit::ui::texturing_tool(&_camera.position, _world.get(), &_show_texture_palette_small_window, _texturing_dock);
  _texturing_dock->setWidget(texturingTool);
  _tool_properties_docks.insert(_texturing_dock);

  _areaid_editor_dock = new QDockWidget("Area ID", this);
  ZoneIDBrowser = new noggit::ui::zone_id_browser(_areaid_editor_dock);
  _areaid_editor_dock->setWidget(ZoneIDBrowser);
  _tool_properties_docks.insert(_areaid_editor_dock);

  _water_editor_dock = new QDockWidget("Water", this);
  guiWater = new noggit::ui::water(&_displayed_water_layer, &_display_all_water_layers, _water_editor_dock);
  _water_editor_dock->setWidget(guiWater);
  _tool_properties_docks.insert(_water_editor_dock);


  connect(guiWater, &noggit::ui::water::regenerate_water_opacity
    , [this](float factor)
    {
      makeCurrent();
      opengl::context::scoped_setter const _(::gl, context());

      _world->autoGenWaterTrans(_camera.position, factor);
    }
  );

  connect(guiWater, &noggit::ui::water::crop_water
    , [this]
    {
      makeCurrent();
      opengl::context::scoped_setter const _(::gl, context());

      _world->CropWaterADT(_camera.position);
    }
  );

  _vertex_shading_dock = new QDockWidget("Vertex Shading", this);
  shaderTool = new noggit::ui::shader_tool(shader_color, _vertex_shading_dock);
  _vertex_shading_dock->setWidget(shaderTool);
  _tool_properties_docks.insert(_vertex_shading_dock);

  _object_editor_dock = new QDockWidget("Object", this);
  objectEditor = new noggit::ui::object_editor(this
    , _world.get()
    , &_move_model_to_cursor_position
    , &_snap_multi_selection_to_ground
    , &_use_median_pivot_point
    , &_object_paste_params
    , _object_editor_dock
  );
  _object_editor_dock->setWidget(objectEditor);
  _tool_properties_docks.insert(_object_editor_dock);


  for (auto widget : _tool_properties_docks)
  {
    connect(this, &QObject::destroyed, widget, &QObject::deleteLater);
  }


  TexturePalette = new noggit::ui::tileset_chooser(this);

  connect( texturingTool->texture_swap_tool()->texture_display()
         , &noggit::ui::current_texture::texture_dropped
         , [=] (std::string const& filename)
           {
             makeCurrent();
             opengl::context::scoped_setter const _(::gl, context());

             texturingTool->texture_swap_tool()->set_texture(filename);
           }
         );

  connect( texturingTool->_current_texture
         , &noggit::ui::current_texture::texture_dropped
         , [=] (std::string const& filename)
           {
             makeCurrent();
             opengl::context::scoped_setter const _(::gl, context());

             noggit::ui::selected_texture::set(filename);
           }
         );
  connect(texturingTool->_current_texture, &noggit::ui::current_texture::clicked
    , [=]
    {
      TexturePalette->setVisible(!TexturePalette->isVisible());
    }
  );
  connect(TexturePalette, &noggit::ui::tileset_chooser::selected
    , [=](std::string const& filename)
    {
      makeCurrent();
      opengl::context::scoped_setter const _(::gl, context());

      noggit::ui::selected_texture::set(filename);
      texturingTool->_current_texture->set_texture(filename);
    }
  );

  connect(this, &QObject::destroyed, TexturePalette, &QObject::deleteLater);

  _texture_palette_small = new noggit::ui::texture_palette_small (this);
  _texture_palette_small->hide();

  connect(_texture_palette_small, &noggit::ui::texture_palette_small::selected
    , [=](std::string const& filename)
    {
      makeCurrent();
      opengl::context::scoped_setter const _(::gl, context());

      noggit::ui::selected_texture::set(filename);
      texturingTool->_current_texture->set_texture(filename);
    }
  );
  connect(this, &QObject::destroyed, _texture_palette_small, &QObject::deleteLater);

  guidetailInfos = new noggit::ui::detail_infos(this);
  guidetailInfos->hide();
  connect(this, &QObject::destroyed, guidetailInfos, &QObject::deleteLater);

  _keybindings = new noggit::ui::help(this);
  _keybindings->hide(); 
  connect(this, &QObject::destroyed, _keybindings, &QObject::deleteLater);

  TexturePicker = new noggit::ui::texture_picker(texturingTool->_current_texture, this);
  TexturePicker->hide();
  connect(this, &QObject::destroyed, TexturePicker, &QObject::deleteLater);

  connect( TexturePicker
         , &noggit::ui::texture_picker::set_texture
         , [=] (scoped_blp_texture_reference texture)
           {
             makeCurrent();
             opengl::context::scoped_setter const _(::gl, context());
             noggit::ui::selected_texture::set(std::move(texture));
           }
         );
  connect(TexturePicker, &noggit::ui::texture_picker::shift_left
    , [=]
    {
      makeCurrent();
      opengl::context::scoped_setter const _(::gl, context());
      TexturePicker->shiftSelectedTextureLeft();
    }
  );
  connect(TexturePicker, &noggit::ui::texture_picker::shift_right
    , [=]
    {
      makeCurrent();
      opengl::context::scoped_setter const _(::gl, context());
      TexturePicker->shiftSelectedTextureRight();
    }
  );

  ZoneIDBrowser->setMapID(_world->getMapID());
  connect(ZoneIDBrowser, &noggit::ui::zone_id_browser::selected
    , [this](int area_id) { changeZoneIDValue(area_id); }
  );

  terrainTool->storeCursorPos(&_cursor_pos);


  for (auto dock : _tool_properties_docks)
  {
    _main_window->addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);

    connect(dock, &QDockWidget::topLevelChanged
      , this
      , [=] 
      { 
        dock->adjustSize();

        for (auto dock_ : _tool_properties_docks)
        {
          if (dock->isFloating() != dock_->isFloating())
            dock_->setFloating(dock->isFloating());
        }
      }
    );
  }

  if (_settings->value("undock_tool_properties/enabled", 1).toBool())
  {
    for (auto dock : _tool_properties_docks)
    {
      dock->setFloating(true);
      dock->move(_main_window->geometry().topRight().x() - dock->rect().width() - 20, _main_window->geometry().topRight().y() + 40);
    }
  }

  // create quick access texture palette dock

  _texture_palette_dock->setFeatures(QDockWidget::DockWidgetMovable
    | QDockWidget::DockWidgetFloatable
    | QDockWidget::DockWidgetClosable
  );

  _texture_palette_dock->setWidget(_texture_palette_small);
  _texture_palette_dock->setWindowTitle("Quick Access Texture Palette");
  _texture_palette_dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  _texture_palette_dock->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

  _main_window->addDockWidget(Qt::BottomDockWidgetArea, _texture_palette_dock);

  if (_settings->value("undock_small_texture_palette/enabled", 1).toBool())
  {
    _texture_palette_dock->setFloating(true);
    _texture_palette_dock->move(_main_window->geometry().bottomLeft().x() + 50, _main_window->geometry().bottomLeft().y() - 200);
  }

  connect(this, &QObject::destroyed, _texture_palette_dock, &QObject::deleteLater);
   
  // create toolbar

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



  ADD_ACTION (file_menu, "Save current tile", "Ctrl+Shift+S", [this] { save(save_mode::current); });
  ADD_ACTION (file_menu, "Save changed tiles", QKeySequence::Save, [this] { save(save_mode::changed); });
  ADD_ACTION (file_menu, "Save all tiles", "Ctrl+Shift+A", [this] { save(save_mode::all); });
  
  ADD_ACTION ( file_menu
             , "Reload tile"
             , "Shift+J"
             , [this]
               {
                 makeCurrent();
                 opengl::context::scoped_setter const _ (::gl, context());
                 _world->reload_tile (_camera.position);
                 _rotation_editor_need_update = true;
               }
             );

  file_menu->addSeparator();
  ADD_ACTION_NS (file_menu, "Force uid check on next opening", [this] { _force_uid_check = true; });
  file_menu->addSeparator();


  edit_menu->addSeparator();
  edit_menu->addAction(createTextSeparator("Selected object"));
  edit_menu->addSeparator();
  ADD_ACTION (edit_menu, "Delete", Qt::Key_Delete, [this] { DeleteSelectedObject(); });
  ADD_ACTION (edit_menu, "Reset rotation", "Ctrl+R", [this] { ResetSelectedObjectRotation(); });
  ADD_ACTION (edit_menu, "Set to ground", Qt::Key_PageDown, [this] { snap_selected_models_to_the_ground(); });


  edit_menu->addSeparator();
  edit_menu->addAction(createTextSeparator("Options"));
  edit_menu->addSeparator();
  ADD_TOGGLE_NS (edit_menu, "Locked cursor mode", _locked_cursor_mode);


  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Model"));
  assist_menu->addSeparator();
  ADD_ACTION (assist_menu, "Last M2 from WMV", "Shift+V", [this] { objectEditor->import_last_model_from_wmv(eEntry_Model); });
  ADD_ACTION (assist_menu, "Last WMO from WMV", "Alt+V", [this] { objectEditor->import_last_model_from_wmv(eEntry_WMO); });
  ADD_ACTION_NS (assist_menu, "Helper models", [this] { objectEditor->helper_models_widget->show(); });

  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Current ADT"));
  assist_menu->addSeparator();
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
                , "Remove texture duplicates"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->removeTexDuplicateOnADT(_camera.position);
                  }
                );
  ADD_ACTION_NS ( assist_menu
                , "Clear textures"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->clearTextures(_camera.position);
                  }
                );
  ADD_ACTION_NS ( assist_menu
                , "Clear textures + set base"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->setBaseTexture(_camera.position);
                  }
                );
    ADD_ACTION_NS ( assist_menu
                , "Clear shadows"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->clear_shadows(_camera.position);
                  }
                );
  ADD_ACTION_NS ( assist_menu
                , "Clear models"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->clearAllModelsOnADT(_camera.position);
                    _rotation_editor_need_update = true;
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

  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Loaded ADTs"));
  assist_menu->addSeparator();
  ADD_ACTION_NS ( assist_menu
                , "Fix gaps"
                , [this]
                  {
                    makeCurrent();
                    opengl::context::scoped_setter const _ (::gl, context());
                    _world->fixAllGaps();
                  }
                );

  assist_menu->addSeparator();
  assist_menu->addAction(createTextSeparator("Global"));
  assist_menu->addSeparator();
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

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Drawing"));
  view_menu->addSeparator();
  ADD_TOGGLE (view_menu, "Doodads", Qt::Key_F1, _draw_models);
  ADD_TOGGLE (view_menu, "WMO doodads", Qt::Key_F2, _draw_wmo_doodads);
  ADD_TOGGLE (view_menu, "Terrain", Qt::Key_F3, _draw_terrain);
  ADD_TOGGLE (view_menu, "Water", Qt::Key_F4, _draw_water);
  ADD_TOGGLE (view_menu, "WMOs", Qt::Key_F6, _draw_wmo);
  ADD_TOGGLE (view_menu, "Lines", Qt::Key_F7, _draw_lines);
  ADD_TOGGLE (view_menu, "Map contour infos", Qt::Key_F9, _draw_contour);
  ADD_TOGGLE (view_menu, "Wireframe", Qt::Key_F10, _draw_wireframe);
  ADD_TOGGLE (view_menu, "Toggle Animation", Qt::Key_F11, _draw_model_animations);
  ADD_TOGGLE (view_menu, "Draw fog", Qt::Key_F12, _draw_fog);
  ADD_TOGGLE_NS (view_menu, "Flight Bounds", _draw_mfbo);
  ADD_TOGGLE (view_menu, "Hole lines always on", "Shift+F7", _draw_hole_lines);
  ADD_TOGGLE_NS (view_menu, "Models with box", _draw_models_with_box);
  //! \todo space+h in object mode
  ADD_TOGGLE_NS (view_menu, "Draw hidden models", _draw_hidden_models);

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Minimap"));
  view_menu->addSeparator();

  ADD_TOGGLE (view_menu, "Show", Qt::Key_M, _show_minimap_window);
  connect ( &_show_minimap_window, &noggit::bool_toggle_property::changed
          , _minimap_dock, [this]
                           {
                             if (!ui_hidden)
                               _minimap_dock->setVisible(_show_minimap_window.get());
                           }
          );
  connect ( _minimap_dock, &QDockWidget::visibilityChanged
          , &_show_minimap_window, &noggit::bool_toggle_property::set
          );

  ADD_TOGGLE_NS(view_menu, "Show ADT borders", _show_minimap_borders);
  connect ( &_show_minimap_borders, &noggit::bool_toggle_property::changed
          , [this]
            {
              _minimap->draw_boundaries(_show_minimap_borders.get());
            }
          );

  ADD_TOGGLE_NS(view_menu, "Show light zones", _show_minimap_skies);
  connect ( &_show_minimap_skies, &noggit::bool_toggle_property::changed
          , [this]
            {
              _minimap->draw_skies(_show_minimap_skies.get());
            }
          );

  view_menu->addSeparator();
  view_menu->addAction(createTextSeparator("Windows"));
  view_menu->addSeparator();


  auto hide_widgets = [=] {

    QWidget* widget_list[] =
    {
      TexturePalette,
      TexturePicker,
      guidetailInfos,
      _cursor_switcher.get(),
      _keybindings,
      _minimap_dock,
      objectEditor->modelImport,
      objectEditor->rotationEditor,
      objectEditor->helper_models_widget,
      _texture_palette_small
    };


    if (_main_window->displayed_widgets.empty())
    {
      for (auto widget : widget_list)
        if (widget->isVisible())
        {
          _main_window->displayed_widgets.emplace(widget);
          widget->hide();
        }

    }
    else
    {
      for (auto widget : _main_window->displayed_widgets)
        widget->show();

      _main_window->displayed_widgets.clear();
    }


    _main_window->statusBar()->setVisible(ui_hidden);
    _toolbar->setVisible(ui_hidden);

    ui_hidden = !ui_hidden;
    
    setToolPropertyWidgetVisibility(terrainMode);

  };

  ADD_ACTION(view_menu, "Toggle UI", Qt::Key_Tab, hide_widgets);


  ADD_TOGGLE (view_menu, "Detail infos", Qt::Key_F8, _show_detail_info_window);
  connect ( &_show_detail_info_window, &noggit::bool_toggle_property::changed
          , guidetailInfos, [this]
                            {
                              if (!ui_hidden)
                                guidetailInfos->setVisible(_show_detail_info_window.get());
                            }
          );
  connect ( guidetailInfos, &noggit::ui::widget::visibilityChanged
          , &_show_detail_info_window, &noggit::bool_toggle_property::set
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
          , TexturePalette, [this] 
                            { 
                              if (terrainMode == editing_mode::paint && !ui_hidden)
                              {
                                TexturePalette->setVisible(_show_texture_palette_window.get());
                              }
                              else
                              {
                                _show_texture_palette_window.set(false);
                              }
                            }
          );
  connect ( TexturePalette, &noggit::ui::widget::visibilityChanged
          , &_show_texture_palette_window, &noggit::bool_toggle_property::set
          );

  ADD_TOGGLE_NS(view_menu, "Small texture palette", _show_texture_palette_small_window);

  addHotkey( Qt::Key_H
           , MOD_none
           , [this] { _show_texture_palette_small_window.toggle(); }
           , [this] { return terrainMode == editing_mode::paint; }
           );

  connect(&_show_texture_palette_small_window, &noggit::bool_toggle_property::changed
    , _texture_palette_dock, [this]
    {
      if (terrainMode == editing_mode::paint && !ui_hidden)
      {
        _texture_palette_dock->setVisible(_show_texture_palette_small_window.get());
      }
      else
      {
        _show_texture_palette_small_window.set(false);
      }
    }
  );
  connect(_texture_palette_dock, &QDockWidget::visibilityChanged
    , &_show_texture_palette_small_window, &noggit::bool_toggle_property::set
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
          , _keybindings, &QWidget::setVisible
          );
  connect ( _keybindings, &noggit::ui::widget::visibilityChanged
          , &_show_keybindings_window, &noggit::bool_toggle_property::set
          );

#if defined(_WIN32) || defined(WIN32)
  ADD_ACTION_NS ( help_menu
                , "Forum"
                , []
                  {
                    ShellExecute ( nullptr
                                 , "open"
                                 , "http://www.modcraft.io/index.php?board=48.0"
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
                                 , "https://bitbucket.org/berndloerwald/noggit3/"
                                 , nullptr
                                 , nullptr
                                 , SW_SHOWNORMAL
                                 );
                  }
                );

  ADD_ACTION_NS ( help_menu
                , "Discord"
                , []
                  {
                    ShellExecute ( nullptr
                                 , "open"
                                 , "https://discord.gg/UbdFHyM"
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

  ADD_ACTION (view_menu, "Increase time speed", Qt::Key_N, [this] { mTimespeed += 90.0f; });
  ADD_ACTION (view_menu, "Decrease time speed", Qt::Key_B, [this] { mTimespeed = std::max (0.0f, mTimespeed - 90.0f); });
  ADD_ACTION (view_menu, "Pause time", Qt::Key_J, [this] { mTimespeed = 0.0f; });

  addHotkey ( Qt::Key_C
            , MOD_ctrl
            , [this]
              {
                objectEditor->copy_current_selection(_world.get());
              }
            , [this] { return terrainMode == editing_mode::object; }
            );
  addHotkey ( Qt::Key_C
            , MOD_none
            , [this]
              {
                objectEditor->copy_current_selection(_world.get());
              }
            , [this] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_C
            , MOD_shift
            , [this]
              {
                do
                {
                  cursor_type.set ((cursor_type.get() + 1) % static_cast<unsigned int>(cursor_mode::mode_count));
                } while (cursor_type.get() == static_cast<unsigned int>(cursor_mode::unused)); // hack to not get the unused cursor type
              }
            , [this] { return terrainMode != editing_mode::object; }
            );

  addHotkey ( Qt::Key_V
            , MOD_ctrl
            , [this] { objectEditor->pasteObject (_cursor_pos, _camera.position, _world.get(), &_object_paste_params); }
            , [this] { return terrainMode == editing_mode::object; }
            );
  addHotkey ( Qt::Key_V
            , MOD_none
            , [this] { objectEditor->pasteObject (_cursor_pos, _camera.position, _world.get(), &_object_paste_params); }
            , [this] { return terrainMode == editing_mode::object; }
            );
  addHotkey ( Qt::Key_V
            , MOD_shift
            , [this] { objectEditor->import_last_model_from_wmv(eEntry_Model); }
            , [this] { return terrainMode == editing_mode::object; }
            );
  addHotkey ( Qt::Key_V
            , MOD_alt
            , [this] { objectEditor->import_last_model_from_wmv(eEntry_WMO); }
            , [this] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_C
            , MOD_none
            , [this] { _world->clearVertexSelection(); }
            , [this] { return terrainMode == editing_mode::ground; }
            );

  addHotkey( Qt::Key_B
	         , MOD_ctrl
	         , [this] 
             {
               objectEditor->copy_current_selection(_world.get());
	             objectEditor->pasteObject(_cursor_pos, _camera.position, _world.get(), &_object_paste_params);
             }
           , [this] { return terrainMode == editing_mode::object; }
           );

  ADD_ACTION (view_menu, "Invert mouse", "I", [this] { mousedir *= -1.f; });

  ADD_ACTION (view_menu, "Decrease camera speed", Qt::Key_O, [this] { _camera.move_speed *= 0.5f; });
  ADD_ACTION (view_menu, "Increase camera speed", Qt::Key_P, [this] { _camera.move_speed *= 2.0f; });

  ADD_ACTION (file_menu, "Save minimaps", "Ctrl+Shift+P", [this] { Saving = true; });

  ADD_ACTION ( view_menu
             , "Turn camera around 180°"
             , "Shift+R"
             , [this]
               {
                 _camera.add_to_yaw(math::degrees(180.f));
                 _camera_moved_since_last_draw = true;
                 _minimap->update();
               }
             );

  ADD_ACTION ( file_menu
             , "Write coordinates to port.txt"
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
             , "Toggle tile mode"
             , Qt::Key_U
             , [this]
               {
                 if (_display_mode == display_mode::in_2D)
                 {
                   _display_mode = display_mode::in_3D;
                   set_editing_mode (saveterrainMode);
                 }
                 else
                 {
                   _display_mode = display_mode::in_2D;
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
                texturingTool->toggle_tool();
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
                if (_world->has_selection())
                {
                  for (auto& selection : _world->current_selection())
                  {
                    if (selection.which() == eEntry_Model)
                    {
                      boost::get<selected_model_type>(selection)->model->toggle_visibility();
                    }
                    else if (selection.which() == eEntry_WMO)
                    {
                      boost::get<selected_wmo_type>(selection)->wmo->toggle_visibility();
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
                _draw_hidden_models.toggle();
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey(Qt::Key_R
	  , MOD_space
	  , [&]
  {
	  texturingTool->toggle_brush_level_min_max();
  }
  , [&] { return terrainMode == editing_mode::paint; }
  );

  addHotkey ( Qt::Key_H
            , MOD_shift
            , [&]
              {
                ModelManager::clear_hidden_models();
                WMOManager::clear_hidden_wmos();
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( Qt::Key_F
            , MOD_space
            , [&]
              {
                terrainTool->flattenVertices (_world.get());
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
                _world->set_selected_models_pos(_cursor_pos);
                _rotation_editor_need_update = true;
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

  addHotkey(Qt::Key_1, MOD_none, [this] { set_editing_mode(editing_mode::ground); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_2, MOD_none, [this] { set_editing_mode (editing_mode::flatten_blur); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_3, MOD_none, [this] { set_editing_mode (editing_mode::paint); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_4, MOD_none, [this] { set_editing_mode (editing_mode::holes); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_5, MOD_none, [this] { set_editing_mode (editing_mode::areaid); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_6, MOD_none, [this] { set_editing_mode (editing_mode::flags); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_7, MOD_none, [this] { set_editing_mode (editing_mode::water); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_8, MOD_none, [this] { set_editing_mode (editing_mode::mccv); }, [this] { return !_mod_num_down;  });
  addHotkey (Qt::Key_9, MOD_none, [this] { set_editing_mode (editing_mode::object); }, [this] { return !_mod_num_down;  });

  addHotkey(Qt::Key_0, MOD_ctrl, [this] { change_selected_wmo_doodadset(0); });
  addHotkey(Qt::Key_1, MOD_ctrl, [this] { change_selected_wmo_doodadset(1); });
  addHotkey(Qt::Key_2, MOD_ctrl, [this] { change_selected_wmo_doodadset(2); });
  addHotkey(Qt::Key_3, MOD_ctrl, [this] { change_selected_wmo_doodadset(3); });
  addHotkey(Qt::Key_4, MOD_ctrl, [this] { change_selected_wmo_doodadset(4); });
  addHotkey(Qt::Key_5, MOD_ctrl, [this] { change_selected_wmo_doodadset(5); });
  addHotkey(Qt::Key_6, MOD_ctrl, [this] { change_selected_wmo_doodadset(6); });
  addHotkey(Qt::Key_7, MOD_ctrl, [this] { change_selected_wmo_doodadset(7); });
  addHotkey(Qt::Key_8, MOD_ctrl, [this] { change_selected_wmo_doodadset(8); });
  addHotkey(Qt::Key_9, MOD_ctrl, [this] { change_selected_wmo_doodadset(9); });

  connect(_main_window, &noggit::ui::main_window::exit_prompt_opened, this, &MapView::on_exit_prompt);
}

void MapView::on_exit_prompt()
{
  // hide all popups
  _cursor_switcher->hide();
  _keybindings->hide();
  _minimap_dock->hide();
  _texture_palette_small->hide();
  objectEditor->helper_models_widget->hide();
  objectEditor->modelImport->hide();
  objectEditor->rotationEditor->hide();
  guidetailInfos->hide();
  TexturePicker->hide();
  TexturePalette->hide();
}

MapView::MapView( math::degrees camera_yaw0
                , math::degrees camera_pitch0
                , math::vector_3d camera_pos
                , noggit::ui::main_window* main_window
                , std::unique_ptr<World> world
                , uid_fix_mode uid_fix
                , bool from_bookmark
                )
  : _camera (camera_pos, camera_yaw0, camera_pitch0)
  , mTimespeed(0.0f)
  , _uid_fix (uid_fix)
  , _from_bookmark (from_bookmark)
  , _settings (new QSettings (this))
  , cursor_color (1.f, 1.f, 1.f, 1.f)
  , shader_color (1.f, 1.f, 1.f, 1.f)
  , cursor_type (static_cast<unsigned int>(cursor_mode::terrain))
  , _main_window (main_window)
  , _world (std::move (world))
  , _status_position (new QLabel (this))
  , _status_selection (new QLabel (this))
  , _status_area (new QLabel (this))
  , _status_time (new QLabel (this))
  , _status_fps (new QLabel (this))
  , _minimap (new noggit::ui::minimap_widget (nullptr))
  , _minimap_dock (new QDockWidget ("Minimap", this))
  , _texture_palette_dock(new QDockWidget(this))
{
  _main_window->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  _main_window->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  _main_window->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  _main_window->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

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
  _main_window->statusBar()->addWidget (_status_fps);
  connect ( this
          , &QObject::destroyed
          , _main_window
          , [=] { _main_window->statusBar()->removeWidget (_status_fps); }
          );

  _minimap->world (_world.get());
  _minimap->camera (&_camera);
  _minimap->draw_boundaries (_show_minimap_borders.get());
  _minimap->draw_skies (_show_minimap_skies.get());

  connect ( _minimap, &noggit::ui::minimap_widget::map_clicked
          , [this] (math::vector_3d const& pos)
            {
              move_camera_with_auto_height (pos);
            }
          );

  _minimap_dock->setFeatures ( QDockWidget::DockWidgetMovable
                             | QDockWidget::DockWidgetFloatable
                             | QDockWidget::DockWidgetClosable
                             );
  _minimap_dock->setWidget(_minimap);
  _main_window->addDockWidget (Qt::RightDockWidgetArea, _minimap_dock);
  _minimap_dock->setVisible (false);
  _minimap_dock->setFloating(true);
  _minimap_dock->move(_main_window->rect().center() - _minimap->rect().center());


  connect(this, &QObject::destroyed, _minimap_dock, &QObject::deleteLater);
  connect(this, &QObject::destroyed, _minimap, &QObject::deleteLater);

  setWindowTitle ("Noggit Studio - " STRPRODUCTVER);

  cursor_type.set (_settings->value ("cursor/default_type", static_cast<unsigned int>(cursor_mode::terrain)).toUInt());

  cursor_color.x = _settings->value ("cursor/color/r", 1).toFloat();
  cursor_color.y = _settings->value ("cursor/color/g", 1).toFloat();
  cursor_color.z = _settings->value ("cursor/color/b", 1).toFloat();
  cursor_color.w = _settings->value ("cursor/color/a", 1).toFloat();

  connect(&cursor_type, &noggit::unsigned_int_property::changed, [&] (unsigned int type)
  {
    _settings->setValue("cursor/default_type", type);
  });

  if (cursor_type.get() == static_cast<unsigned int>(cursor_mode::unused))
  {
    cursor_type.set(static_cast<unsigned int>(cursor_mode::terrain));
  }

  _cursor_switcher.reset(new noggit::ui::cursor_switcher (this, cursor_color, cursor_type));

  setFocusPolicy (Qt::StrongFocus);
  setMouseTracking (true);

  moving = strafing = updown = lookat = turn = 0.0f;

  freelook = false;

  mousedir = -1.0f;

  look = false;
  _display_mode = display_mode::in_3D;

  _tablet_active = true;

  _startup_time.start();
  _update_every_event_loop.start (0);
  connect (&_update_every_event_loop, &QTimer::timeout, [this] { update(); });
}

void MapView::tabletEvent(QTabletEvent* event)
{
  _tablet_pressure = event->pressure();
  event->setAccepted(true);
    
}

void MapView::move_camera_with_auto_height (math::vector_3d const& pos)
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  _world->mapIndex.loadTile(pos)->wait_until_loaded();

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

void MapView::on_uid_fix_fail()
{
  emit uid_fix_failed();

  _uid_fix_failed = true;
  deleteLater();
}

void MapView::initializeGL()
{
  bool uid_warning = false;

  opengl::context::scoped_setter const _ (::gl, context());
  gl.viewport(0.0f, 0.0f, width(), height());

  gl.clearColor (0.0f, 0.0f, 0.0f, 1.0f);

  if (_uid_fix == uid_fix_mode::max_uid)
  {
    _world->mapIndex.searchMaxUID();
  }
  else if (_uid_fix == uid_fix_mode::fix_all_fail_on_model_loading_error)
  {
    auto result = _world->mapIndex.fixUIDs (_world.get(), true);

    if (result == uid_fix_status::failed)
    {
      on_uid_fix_fail();
      return;
    }
  }
  else if (_uid_fix == uid_fix_mode::fix_all_fuckporting_edition)
  {
    auto result = _world->mapIndex.fixUIDs (_world.get(), false);

    uid_warning = result == uid_fix_status::done_with_errors;
  }

  _uid_fix = uid_fix_mode::none;

  createGUI();

  set_editing_mode (editing_mode::ground);

  if (!_from_bookmark)
  {
    move_camera_with_auto_height (_camera.position);
  }

  if (uid_warning)
  {
    QMessageBox::warning
      ( nullptr
      , "UID Warning"
      , "Some models were missing or couldn't be loaded. "
        "This will lead to culling (visibility) errors in game\n"
        "It is recommanded to fix those models (listed in the log file) and run the uid fix all again."
      , QMessageBox::Ok
      );
  }
}

void MapView::paintGL()
{
  opengl::context::scoped_setter const _ (::gl, context());
  const qreal now(_startup_time.elapsed() / 1000.0);

  _last_frame_durations.emplace_back (now - _last_update);

  tick (now - _last_update);
  _last_update = now;

  gl.clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_map();

  if (_world->uid_duplicates_found() && !_uid_duplicate_warning_shown)
  {
    _uid_duplicate_warning_shown = true;

    QMessageBox::critical( this
                          , "UID ALREADY IN USE"
                          , "Please enable 'Always check for max UID', mysql uid store or synchronize your "
                            "uid.ini file if you're sharing the map between several mappers.\n\n"
                            "Use 'Editor > Force uid check on next opening' to fix the issue."
                          );
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

  // when the uid fix fail the UI isn't created
  if (!_uid_fix_failed)
  {
    delete TexturePicker; // explicitly delete this here to avoid opengl context related crash
    delete objectEditor;
    delete texturingTool;
  }
  
  if (_force_uid_check)
  {
    uid_storage::remove_uid_for_map(_world->getMapID());
  }

  _world.reset();

  AsyncLoader::instance().reset_object_fail();

  noggit::ui::selected_texture::texture.reset();

  ModelManager::report();
  TextureManager::report();
  WMOManager::report();
}

void MapView::tick (float dt)
{
	_mod_shift_down = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
	_mod_ctrl_down = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
	_mod_alt_down = QApplication::keyboardModifiers().testFlag(Qt::AltModifier);
	_mod_num_down = QApplication::keyboardModifiers().testFlag(Qt::KeypadModifier);


  // start unloading tiles
  _world->mapIndex.enterTile (tile_index (_camera.position));
  _world->mapIndex.unloadTiles (tile_index (_camera.position));

  dt = std::min(dt, 1.0f);

  if (_locked_cursor_mode.get())
  {
    switch (terrainMode)
    {
      case editing_mode::areaid:
      case editing_mode::flags:
      case editing_mode::holes:
      case editing_mode::object:
        update_cursor_pos();
        break;
    }    
  }
  else
  {
    update_cursor_pos();
  }

  if (_tablet_active && _settings->value ("tablet/enabled", false).toBool())
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      terrainTool->setSpeed(_tablet_pressure * 10.0f);
    case editing_mode::flatten_blur:
      flattenTool->setSpeed(_tablet_pressure * 10.0f);
      break;
    case editing_mode::paint:
      texturingTool->set_pressure(_tablet_pressure);
      break;
    }
  }

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

  auto currentSelection = _world->current_selection();
  if (_world->has_selection())
  {
    // update rotation editor if the selection has changed
    if (lastSelected != currentSelection)
    {
      _rotation_editor_need_update = true;
    }

    if (terrainMode == editing_mode::object)
    {
      // reset numpad_moveratio when no numpad key is pressed
      if (!(keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0))
      {
        numpad_moveratio = 0.001f;
      }
      else // Set move scale and rotate for numpad keys
      {
        if (_mod_ctrl_down && _mod_shift_down)
        {
          numpad_moveratio += 0.1f;
        }
        else if (_mod_shift_down)
        {
          numpad_moveratio += 0.01f;
        }
        else if (_mod_ctrl_down)
        {
          numpad_moveratio += 0.0005f;
        }
      }

      if (keys != 0.f)
      {
        _world->scale_selected_models(keys*numpad_moveratio / 50.f, World::m2_scaling_type::add);
        _rotation_editor_need_update = true;
      }
      if (keyr != 0.f)
      {
        _world->rotate_selected_models( math::degrees(0.f)
                                      , math::degrees(keyr * numpad_moveratio * 5.f)
                                      , math::degrees(0.f)
                                      , _use_median_pivot_point.get()
                                      );
        _rotation_editor_need_update = true;
      }

      if (MoveObj)
      {
        if (_mod_alt_down)
        {
          _world->scale_selected_models(std::pow(2.f, mv*4.f), World::m2_scaling_type::mult);
        }
        else if (_mod_shift_down)
        {
          _world->move_selected_models(0.f, mv*80.f, 0.f);
        }
        else
        {
          if (_world->has_multiple_model_selected())
          {
            _world->set_selected_models_pos(_cursor_pos, false);

            if (_snap_multi_selection_to_ground.get())
            {
              snap_selected_models_to_the_ground();
            }
          }
          else
          {
            if (!_move_model_to_cursor_position.get())
            {
              _world->move_selected_models((mv * dirUp - mh * dirRight)*80.f);
            }
            else
            {
              _world->set_selected_models_pos(_cursor_pos, false);
            }
          }
        }

        _rotation_editor_need_update = true;
      }

      if (keyx != 0.f || keyy != 0.f || keyz != 0.f)
      {
        _world->move_selected_models(keyx * numpad_moveratio, keyy * numpad_moveratio, keyz * numpad_moveratio);
        _rotation_editor_need_update = true;
      }

      if (look)
      {
        if (_mod_ctrl_down) // X
        {
          _world->rotate_selected_models( math::degrees(rh + rv)
                                        , math::degrees(0.f)
                                        , math::degrees(0.f)
                                        , _use_median_pivot_point.get()
                                        );
        }
        if (_mod_shift_down) // Y
        {
          _world->rotate_selected_models( math::degrees(0.f)
                                        , math::degrees(rh + rv)
                                        , math::degrees(0.f)
                                        , _use_median_pivot_point.get()
                                        );
        }
        if (_mod_alt_down) // Z
        {
          _world->rotate_selected_models( math::degrees(0.f)
                                        , math::degrees(0.f)
                                        , math::degrees(rh + rv)
                                        , _use_median_pivot_point.get()
                                        );
        }

        _rotation_editor_need_update = true;
      }
    }

    for (auto& selection : currentSelection)
    {
      if (leftMouse && selection.which() == eEntry_MapChunk)
      {
        bool underMap = _world->isUnderMap(_cursor_pos);

        switch (terrainMode)
        {
        case editing_mode::ground:
          if (_display_mode == display_mode::in_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              terrainTool->changeTerrain(_world.get(), _cursor_pos, 7.5f * dt);
            }
            else if (_mod_ctrl_down)
            {
              terrainTool->changeTerrain(_world.get(), _cursor_pos, -7.5f * dt);
            }
          }
          break;
        case editing_mode::flatten_blur:
          if (_display_mode == display_mode::in_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              flattenTool->flatten(_world.get(), _cursor_pos, dt);
            }
            else if (_mod_ctrl_down)
            {
              flattenTool->blur(_world.get(), _cursor_pos, dt);
            }
          }
          break;
        case editing_mode::paint:
          if (_mod_shift_down && _mod_ctrl_down && _mod_alt_down)
          {
            // clear chunk texture
            if (!underMap)
            {
              _world->eraseTextures(_cursor_pos);
            }
          }
          else if (_mod_ctrl_down && !ui_hidden)
          {
            // Pick texture
            TexturePicker->getTextures(selection);
          }
          else  if (_mod_shift_down && !!noggit::ui::selected_texture::get())
          {
            if ((_display_mode == display_mode::in_3D && !underMap) || _display_mode == display_mode::in_2D)
            {
              texturingTool->paint(_world.get(), _cursor_pos, dt, *noggit::ui::selected_texture::get());
            }
          }
          break;

        case editing_mode::holes:
          // no undermap check here, else it's impossible to remove holes
          if (_mod_shift_down)
          {
            _world->setHole(_cursor_pos, _mod_alt_down, false);
          }
          else if (_mod_ctrl_down && !underMap)
          {
            _world->setHole(_cursor_pos, _mod_alt_down, true);
          }
          break;
        case editing_mode::areaid:
          if (!underMap)
          {
            if (_mod_shift_down)
            {
              // draw the selected AreaId on current selected chunk
              _world->setAreaID(_cursor_pos, _selected_area_id, false);
            }
            else if (_mod_ctrl_down)
            {
              // pick areaID from chunk
              MapChunk* chnk(boost::get<selected_chunk_type>(selection).chunk);
              int newID = chnk->getAreaID();
              _selected_area_id = newID;
              ZoneIDBrowser->setZoneID(newID);
            }
          }
          break;
        case editing_mode::flags:
          if (!underMap)
          {
            // todo: replace this
            if (_mod_shift_down)
            {
              _world->mapIndex.setFlag(true, _cursor_pos, 0x2);
            }
            else if (_mod_ctrl_down)
            {
              _world->mapIndex.setFlag(false, _cursor_pos, 0x2);
            }
          }
          break;
        case editing_mode::water:
          if (_display_mode == display_mode::in_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              guiWater->paintLiquid(_world.get(), _cursor_pos, true);
            }
            else if (_mod_ctrl_down)
            {
              guiWater->paintLiquid(_world.get(), _cursor_pos, false);
            }
          }
          break;
        case editing_mode::mccv:
          if (!underMap)
          {
            if (_mod_shift_down)
            {
              shaderTool->changeShader(_world.get(), _cursor_pos, dt, true);
            }
            if (_mod_ctrl_down)
            {
              shaderTool->changeShader(_world.get(), _cursor_pos, dt, false);
            }
          }
          break;
        }
      }
    }
  }

  mh = 0;
  mv = 0;
  rh = 0;
  rv = 0;

  if (_display_mode != display_mode::in_2D)
  {
    if (turn)
    {
      _camera.add_to_yaw(math::degrees(turn));
      _camera_moved_since_last_draw = true;
    }
    if (lookat)
    {
      _camera.add_to_pitch(math::degrees(lookat));
      _camera_moved_since_last_draw = true;
    }
    if (moving)
    {
      _camera.move_forward(moving, dt);
      _camera_moved_since_last_draw = true;
    }
    if (strafing)
    {
      _camera.move_horizontal(strafing, dt);
      _camera_moved_since_last_draw = true;
    }
    if (updown)
    {
      _camera.move_vertical(updown, dt);
      _camera_moved_since_last_draw = true;
    }
  }
  else
  {
    //! \todo this is total bullshit. there should be a seperate view and camera class for tilemode
    if (moving)
    {
      _camera.position.z -= dt * _camera.move_speed * moving;
      _camera_moved_since_last_draw = true;
    }
    if (strafing)
    {
      _camera.position.x += dt * _camera.move_speed * strafing;
      _camera_moved_since_last_draw = true;
    }
    if (updown)
    {
      _2d_zoom *= pow(2.0f, dt * updown * 4.0f);
      _2d_zoom = std::max(0.01f, _2d_zoom);
      _camera_moved_since_last_draw = true;
    }
  }

  _minimap->update();

  _world->time += this->mTimespeed * dt;
  _world->animtime += dt * 1000.0f;

  if (_draw_model_animations.get())
  {
    _world->update_models_emitters(dt);
  }

  if (_world->has_selection())
  {
    lastSelected = currentSelection;
  }

  if (_rotation_editor_need_update)
  {
    objectEditor->rotationEditor->updateValues(_world.get());
    _rotation_editor_need_update = false;
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

  if (currentSelection.size() > 0)
  {
    _status_selection->setText ("");
  }
  else if (currentSelection.size() == 1)
  {
    switch (currentSelection.begin()->which())
    {
    case eEntry_Model:
      {
      auto instance(boost::get<selected_model_type>(*currentSelection.begin()));
        _status_selection->setText
          ( QString ("%1: %2")
          . arg (instance->uid)
          . arg (QString::fromStdString (instance->model->filename))
          );
        break;
      }
    case eEntry_WMO:
      {
      auto instance(boost::get<selected_wmo_type>(*currentSelection.begin()));
        _status_selection->setText
          ( QString ("%1: %2")
          . arg (instance->mUniqueID)
          . arg (QString::fromStdString (instance->wmo->filename))
          );
        break;
      }
    case eEntry_MapChunk:
      {
      auto chunk(boost::get<selected_chunk_type>(*currentSelection.begin()).chunk);
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


    if (_tablet_active && _settings->value ("tablet/enabled", false).toBool())
    {
      timestrs << ", Pres: " << _tablet_pressure;
    }

    _status_time->setText (QString::fromStdString (timestrs.str()));
  }

  _last_fps_update += dt;

  // update fps every sec
  if (_last_fps_update > 1.f && !_last_frame_durations.empty())
  {
    auto avg_frame_duration
      ( std::accumulate ( _last_frame_durations.begin()
                        , _last_frame_durations.end()
                        , 0.
                        )
      / qreal (_last_frame_durations.size())
      );
    _status_fps->setText ( "FPS: " + QString::number (int (1. / avg_frame_duration)) 
                         + " - Average frame time: " + QString::number(avg_frame_duration*1000.0) + "ms"
                         );

    _last_frame_durations.clear();
    _last_fps_update = 0.f;
  }

  guiWater->updatePos (_camera.position);

  
  if (guidetailInfos->isVisible())
  {
    if(currentSelection.size() > 0)
    {
      std::stringstream select_info;
      auto lastSelection = currentSelection.back();

      switch (lastSelection.which())
      {
      case eEntry_Model:
        {
        auto instance(boost::get<selected_model_type>(lastSelection));
          select_info << "filename: " << instance->model->filename
                      << "\nunique ID: " << instance->uid
                      << "\nposition X/Y/Z: " << instance->pos.x << " / " << instance->pos.y << " / " << instance->pos.z
                      << "\nrotation X/Y/Z: " << instance->dir.x << " / " << instance->dir.y << " / " << instance->dir.z
                      << "\nscale: " << instance->scale
                      << "\ntextures Used: " << instance->model->header.nTextures
                      << "\nsize category: " << instance->size_cat;

          for (unsigned int j = 0; j < std::min(instance->model->header.nTextures, 6U); j++)
          {
            select_info << "\n " << (j + 1) << ": " << instance->model->_textures[j]->filename;
          }
          if (instance->model->header.nTextures > 25)
          {
            select_info << "\n and more.";
          }

          select_info << "\n";
          break;
        }
      case eEntry_WMO:
        {
        auto instance(boost::get<selected_wmo_type>(lastSelection));
          select_info << "filename: " << instance->wmo->filename
                      << "\nunique ID: " << instance->mUniqueID
                      << "\nposition X/Y/Z: " << instance->pos.x << " / " << instance->pos.y << " / " << instance->pos.z
                      << "\nrotation X/Y/Z: " << instance->dir.x << " / " << instance->dir.y << " / " << instance->dir.z
                      << "\ndoodad set: " << instance->doodadset()
                      << "\ntextures used: " << instance->wmo->textures.size();


          const unsigned int texture_count (std::min((unsigned int)(instance->wmo->textures.size()), 8U));
          for (unsigned int j = 0; j < texture_count; j++)
          {
            select_info << "\n " << (j + 1) << ": " << instance->wmo->textures[j]->filename;
          }
          if (instance->wmo->textures.size() > 25)
          {
            select_info << "\n and more.";
          }

          select_info << "\n";
          break;
        }
      case eEntry_MapChunk:
        {
        auto chunk(boost::get<selected_chunk_type>(lastSelection).chunk);
          mcnk_flags const& flags = chunk->header_flags;

          select_info << "MCNK " << chunk->px << ", " << chunk->py << " (" << chunk->py * 16 + chunk->px
                      << ") of tile (" << chunk->mt->index.x << " " << chunk->mt->index.z << ")"
                      << "\narea ID: " << chunk->getAreaID() << " (\"" << gAreaDB.getAreaName(chunk->getAreaID()) << "\")"
                      << "\nflags: "
                      << (flags.flags.has_mcsh ? "shadows " : "")
                      << (flags.flags.impass   ? "impassable " : "")
                      << (flags.flags.lq_river ? "river " : "")
                      << (flags.flags.lq_ocean ? "ocean " : "")
                      << (flags.flags.lq_magma ? "lava" : "")
                      << (flags.flags.lq_slime ? "slime" : "")
                      << "\ntextures used: " << chunk->texture_set->num();

          //! \todo get a list of textures and their flags as well as detail doodads.

          select_info << "\n";

          break;
        }
      }
    
      guidetailInfos->setText(select_info.str());
    }
    else
    {
      guidetailInfos->setText("");
    }
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

math::ray MapView::intersect_ray() const
{
  float mx = _last_mouse_pos.x(), mz = _last_mouse_pos.y();

  if (_display_mode == display_mode::in_3D)
  {
    // during rendering we multiply perspective * view
    // so we need the same order here and then invert.
    math::vector_3d const pos 
    (
      ( ( projection() 
        * model_view()
        ).inverted()
        * normalized_device_coords (mx, mz)
      ).xyz_normalized_by_w()
    );

    return { _camera.position, pos - _camera.position };
  }
  else
  {
    math::vector_3d const pos
    ( _camera.position.x - (width() * 0.5f - mx) * _2d_zoom
    , _camera.position.y
    , _camera.position.z - (height() * 0.5f - mz) * _2d_zoom
    );
    
    return { pos, math::vector_3d(0.f, -1.f, 0.f) };
  }
}

selection_result MapView::intersect_result(bool terrain_only)
{
  selection_result results
  ( _world->intersect 
    ( model_view().transposed()
    , intersect_ray()
    , terrain_only
    , terrainMode == editing_mode::object
    , _draw_terrain.get()
    , _draw_wmo.get()
    , _draw_models.get()
    , _draw_hidden_models.get()
    )
  );

  std::sort ( results.begin()
            , results.end()
            , [](selection_entry const& lhs, selection_entry const& rhs)
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
    _world->reset_selection();
  }
  else
  {
    auto const& hit (results.front().second);

    if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
    {
      if (hit.which() == eEntry_Model || hit.which() == eEntry_WMO)
      {
        if (!_world->is_selected(hit))
        {
          _world->add_to_selection(hit);
        }
        else
        {
          _world->remove_from_selection(hit);
        }
      }
    }
    else
    {
      _world->reset_selection();
      _world->add_to_selection(hit);
    }

    _cursor_pos = hit.which() == eEntry_Model ? boost::get<selected_model_type>(hit)->pos
      : hit.which() == eEntry_WMO ? boost::get<selected_wmo_type>(hit)->pos
      : hit.which() == eEntry_MapChunk ? boost::get<selected_chunk_type>(hit).position
      : throw std::logic_error("bad variant");
  }

  _rotation_editor_need_update = true;
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

math::matrix_4x4 MapView::model_view() const
{
  if (_display_mode == display_mode::in_2D)
  {
    math::vector_3d eye = _camera.position;
    math::vector_3d target = eye;
    target.y -= 1.f;
    target.z -= 0.001f;

    return math::look_at(eye, target, {0.f,1.f, 0.f});
  }
  else
  {
    return _camera.look_at_matrix();
  }
}
math::matrix_4x4 MapView::projection() const
{
  float far_z = _settings->value("farZ", 2048).toFloat();

  if (_display_mode == display_mode::in_2D)
  {
    float half_width = width() * 0.5f * _2d_zoom;
    float half_height = height() * 0.5f * _2d_zoom;

    return math::ortho(-half_width, half_width, -half_height, half_height, -1.f, far_z);
  }
  else
  {
    return math::perspective(_camera.fov(), aspect_ratio(), 1.f, far_z);
  }
}

void MapView::draw_map()
{
  //! \ todo: make the current tool return the radius
  float radius = 0.0f, inner_radius = 0.0f, angle = 0.0f, orientation = 0.0f;
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
    inner_radius = texturingTool->hardness();
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

  //! \note Select terrain below mouse, if no item selected or the item is map.
  if (!(_world->has_selection()
    || _locked_cursor_mode.get()))
  {
    doSelection(true);
  }

  _world->draw ( model_view().transposed()
               , projection().transposed()
               , _cursor_pos
               , terrainMode == editing_mode::mccv ? shader_color : cursor_color
               , cursor_type.get()
               , radius
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
               , terrainMode == editing_mode::areaid
               , terrainMode
               , _camera.position
               , _camera_moved_since_last_draw
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
               , _draw_hidden_models.get()
               , _area_id_colors
               , _draw_fog.get()
               , terrainTool->_edit_type
               , _display_all_water_layers.get() ? -1 : _displayed_water_layer.get()
               , _display_mode
               );

  // reset after each world::draw call
  _camera_moved_since_last_draw = false;
}

void MapView::keyPressEvent (QKeyEvent *event)
{
  size_t const modifier
    ( ((event->modifiers() & Qt::ShiftModifier) ? MOD_shift : 0)
    | ((event->modifiers() & Qt::ControlModifier) ? MOD_ctrl : 0)
    | ((event->modifiers() & Qt::AltModifier) ? MOD_alt : 0)
    | ((event->modifiers() & Qt::MetaModifier) ? MOD_meta : 0)
    | ((event->modifiers() & Qt::KeypadModifier) ? MOD_num : 0)
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

  if (event->key() == Qt::Key_Space)
    _mod_space_down = true;


  // movement
  if (event->key() == Qt::Key_W)
  {
    moving = 1.0f;
  }
  if (event->key() == Qt::Key_S)
  {
    moving = -1.0f;
  }

  if (event->key() == Qt::Key_Up)
  {
    lookat = 0.75f;
  }
  if (event->key() == Qt::Key_Down)
  {
    lookat = -0.75f;
  }

  if (event->key() == Qt::Key_Right)
  {
    turn = 0.75f;
  }
  if (event->key() == Qt::Key_Left)
  {
    turn = -0.75f;
  }

  if (event->key() == Qt::Key_D)
  {
    strafing = 1.0f;
  }
  if (event->key() == Qt::Key_A)
  {
    strafing = -1.0f;
  }

  if (event->key() == Qt::Key_Q)
  {
    updown = 1.0f;
  }
  if (event->key() == Qt::Key_E)
  {
    updown = -1.0f;
  }

  if (event->key() == Qt::Key_2 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx = 1;
  }
  if (event->key() == Qt::Key_8 && event->modifiers() & Qt::KeypadModifier)
  {
    keyx = -1;
  }

  if (event->key() == Qt::Key_4 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz = 1;
  }
  if (event->key() == Qt::Key_6 && event->modifiers() & Qt::KeypadModifier)
  {
    keyz = -1;
  }

  if (event->key() == Qt::Key_3 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy = 1;
  }
  if (event->key() == Qt::Key_1 && event->modifiers() & Qt::KeypadModifier)
  {
    keyy = -1;
  }

  if (event->key() == Qt::Key_7 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr = 1;
  }
  if (event->key() == Qt::Key_9 && event->modifiers() & Qt::KeypadModifier)
  {
    keyr = -1;
  }

  if (event->key() == Qt::Key_Plus)
  {
    keys = 1;

    switch (terrainMode)
    {
      case editing_mode::mccv:
      {
        shaderTool->addColorToPalette();
        break;
      }
    }
  }
  if (event->key() == Qt::Key_Minus)
  {
    keys = -1;
  }
  if (event->key() == Qt::Key_Home)
  {
	  _camera.position = math::vector_3d(_cursor_pos.x, _cursor_pos.y + 50, _cursor_pos.z); ;
	  _minimap->update();
  }

  if (event->key() == Qt::Key_L)
  {
    freelook = true;
  }
}

void MapView::keyReleaseEvent (QKeyEvent* event)
{
  if (event->key() == Qt::Key_Space)
    _mod_space_down = false;

  // movement
  if (event->key() == Qt::Key_W || event->key() == Qt::Key_S)
  {
    moving = 0.0f;
  }

  if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
  {
    lookat = 0.0f;
  }

  if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Left)
  {
    turn  = 0.0f;
  }

  if (event->key() == Qt::Key_D || event->key() == Qt::Key_A)
  {
    strafing  = 0.0f;
  }

  if (event->key() == Qt::Key_Q || event->key() == Qt::Key_E)
  {
    updown  = 0.0f;
  }
  

  if ((event->key() == Qt::Key_2 || event->key() == Qt::Key_8) && event->modifiers() & Qt::KeypadModifier)
  {
    keyx = 0.0f;
  }

  if ((event->key() == Qt::Key_4 || event->key() == Qt::Key_6) && event->modifiers() & Qt::KeypadModifier)
  {
    keyz = 0.0f;
  }

  if ((event->key() == Qt::Key_3 || event->key() == Qt::Key_1) && event->modifiers() & Qt::KeypadModifier)
  {
    keyy = 0.0f;
  }

  if ((event->key() == Qt::Key_7 || event->key() == Qt::Key_9) && event->modifiers() & Qt::KeypadModifier)
  {
    keyr  = 0.0f;
  }

  if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Minus)
  {
    keys = 0.0f;
  }

  if (event->key() == Qt::Key_L || event->key() == Qt::Key_Minus)
  {
    freelook = false;
  }

}

void MapView::focusOutEvent (QFocusEvent*)
{
  _mod_alt_down = false;
  _mod_ctrl_down = false;
  _mod_shift_down = false;
  _mod_space_down = false;
  _mod_num_down = false;

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
  freelook = false;
}

void MapView::mouseMoveEvent (QMouseEvent* event)
{
  //! \todo:  move the function call requiring a context in tick ?
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());
  QLineF const relative_movement (_last_mouse_pos, event->pos());

  if ((look || freelook) && !(_mod_shift_down || _mod_ctrl_down || _mod_alt_down || _mod_space_down))
  {
    _camera.add_to_yaw(math::degrees(relative_movement.dx() / XSENS));
    _camera.add_to_pitch(math::degrees(mousedir * relative_movement.dy() / YSENS));
    _camera_moved_since_last_draw = true;
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
      terrainTool->moveVertices (_world.get(), -relative_movement.dy() / YSENS);
    }
  }


  if (rightMouse && _mod_space_down)
  {
    terrainTool->setOrientRelativeTo (_world.get(), _cursor_pos);
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

  if (_display_mode == display_mode::in_2D && leftMouse && _mod_alt_down && _mod_shift_down)
  {
    strafing = ((relative_movement.dx() / XSENS) / -1) * 5.0f;
    moving = (relative_movement.dy() / YSENS) * 5.0f;
  }

  if (_display_mode == display_mode::in_2D && rightMouse && _mod_shift_down)
  {
    updown = (relative_movement.dy() / YSENS);
  }

  _last_mouse_pos = event->pos();
}

void MapView::selectModel(std::string const& model)
{
  if (boost::ends_with (model, ".m2"))
  {
    ModelInstance mi(model);
    _world->set_current_selection(boost::get<selected_model_type>(&mi));

  }
  else if (boost::ends_with (model, ".wmo"))
  {
    WMOInstance wi(model);
    _world->set_current_selection(boost::get<selected_wmo_type>(&wi));
  }

  objectEditor->copy_current_selection(_world.get());
  _rotation_editor_need_update = true;
}

void MapView::change_selected_wmo_doodadset(int set)
{
  for (auto& selection : _world->current_selection())
  {
    if (selection.which() == eEntry_WMO)
    {
      auto wmo = boost::get<selected_wmo_type>(selection);
      wmo->change_doodadset(set);
      _world->updateTilesWMO(wmo, model_update::none);
    }
  }
}

void MapView::mousePressEvent(QMouseEvent* event)
{
  makeCurrent();
  opengl::context::scoped_setter const _(::gl, context());

  switch (event->button())
  {
  case Qt::LeftButton:
    leftMouse = true;
    break;

  case Qt::RightButton:
    rightMouse = true;
    break;

  case Qt::MiddleButton:
    if (_world->has_selection())
    {
      MoveObj = true;
    }

    if(terrainMode == editing_mode::mccv)
    {
      shaderTool->pickColor(_world.get(), _cursor_pos);
    }

    break;
  }

  if (leftMouse)
  {
    doSelection(false);
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
        return (_mod_ctrl_down ? 0.01f : 0.1f) 
          * range 
          // alt = horizontal delta
          * (_mod_alt_down ? event->angleDelta().x() : event->angleDelta().y())
          / 320.f
          ;
      }
    );

  if (terrainMode == editing_mode::ground)
  {
    if (_mod_shift_down)
    {
      terrainTool->changeAngle (delta_for_range (178.f));
    }
    else if (_mod_alt_down)
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

    if (_display_mode == display_mode::in_2D)
    {
      strafing = 0;
      moving = 0;
    }
    break;

  case Qt::RightButton:
    rightMouse = false;

    look = false;

    if (_display_mode == display_mode::in_2D)
      updown = 0;

    break;

  case Qt::MiddleButton:
    MoveObj = false;
    break;
  }
}

void MapView::save(save_mode mode)
{
  bool save = true;

  if (AsyncLoader::instance().important_object_failed_loading())
  {
    save = false;
    QPushButton *yes, *no;

    QMessageBox first_warning;
    first_warning.setIcon(QMessageBox::Critical);
    first_warning.setWindowIcon(QIcon (":/icon"));
    first_warning.setWindowTitle("Some models couldn't be loaded");
    first_warning.setText("Error:\nSome models could not be loaded and saving will cause collision and culling issues, would you still like to save ?");
    // roles are swapped to force the user to pay attention and both are "accept" roles so that escape does nothing
    no = first_warning.addButton("No", QMessageBox::ButtonRole::AcceptRole);
    yes = first_warning.addButton("Yes", QMessageBox::ButtonRole::YesRole);
    first_warning.setDefaultButton(no);

    first_warning.exec();

    if (first_warning.clickedButton() == yes)
    {
      QMessageBox second_warning;
      second_warning.setIcon(QMessageBox::Warning);
      second_warning.setWindowIcon(QIcon (":/icon"));
      second_warning.setWindowTitle("Are you sure ?");
      second_warning.setText( "If you save you will have to save again all the adt containing the defective/missing models once you've fixed said models to correct all the issues.\n"
                              "By clicking yes you accept to bear all the consequences of your action and forfeit the right to complain to the developers about any culling and collision issues.\n\n"
                              "So... do you REALLY want to save ?"
                            );
      no = second_warning.addButton("No", QMessageBox::ButtonRole::YesRole);
      yes = second_warning.addButton("Yes", QMessageBox::ButtonRole::AcceptRole);
      second_warning.setDefaultButton(no);

      second_warning.exec();

      if (second_warning.clickedButton() == yes)
      {
        save = true;
      }
    }
  }

  if ( mode == save_mode::current 
    && save 
    && (QMessageBox::warning
          (nullptr
          , "Save current map tile only"
          , "This can cause a collision bug when placing objects between two ADT borders!\n\n"
            "We recommend you to use the normal save function rather than "
            "this one to get the collisions right."
          , QMessageBox::Save | QMessageBox::Cancel
          , QMessageBox::Cancel
          ) == QMessageBox::Cancel
       )
     )
  {
    save = false;
  }

  if (save)
  {
    makeCurrent();
    opengl::context::scoped_setter const _ (::gl, context());

    switch (mode)
    {
    case save_mode::current: _world->mapIndex.saveTile(tile_index(_camera.position), _world.get()); break;
    case save_mode::changed: _world->mapIndex.saveChanged(_world.get()); break;
    case save_mode::all:     _world->mapIndex.saveall(_world.get()); break;
    }    

    AsyncLoader::instance().reset_object_fail();


    _main_window->statusBar()->showMessage("Map saved", 2000);

  }
  else
  {
    QMessageBox::warning
      ( nullptr
      , "Map NOT saved"
      , "The map wasn NOT saved, don't forget to save before leaving"
      , QMessageBox::Ok
      );
  }
}

void MapView::addHotkey(Qt::Key key, size_t modifiers, std::function<void()> function, std::function<bool()> condition)
{
  hotkeys.emplace_front (key, modifiers, function, condition);
}
