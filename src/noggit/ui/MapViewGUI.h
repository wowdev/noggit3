// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QDockWidget>

class MapView;
class UIFrame;
class UITexturePicker;
class UIHelp;
class UIWindow;
class UIExitWarning;
class UIHelperModels;
class UIWater;
class UIObjectEditor;
class MapView;
namespace noggit
{
  class camera;
  namespace ui
  {
    class current_texture;
  }
}
namespace ui
{
  class detail_infos;
  class FlattenTool;
  class shader_tool;
  class terrain_tool;
  class texturing_tool;
  class zone_id_browser;
  class water_save_warning;
  class water_type_browser;
}

#include <math/vector_3d.hpp>
#include <noggit/ui/Frame.h>

//! \todo Give better name.
class UIMapViewGUI : public UIFrame
{
private:
  noggit::camera* _camera;
  float* _tablet_pressure;
  bool* _tablet_active;

public:
  UIHelp* _help;
  // position of the tools window(s)
  int tool_settings_x;
  int tool_settings_y;

  // Editor paramter
  int ground_edit_mode;
  int selection_view_mode;

  MapView* theMapview;
  // UI elements
  UIFrame* TexturePalette;
  UIFrame* SelectedTexture;
  ui::detail_infos* guidetailInfos;
  ui::zone_id_browser* ZoneIDBrowser;
  UITexturePicker* TexturePicker;
  UIWater* guiWater;
  ui::water_type_browser* guiWaterTypeSelector;
  noggit::ui::current_texture* guiCurrentTexture;
  UIObjectEditor* objectEditor;
  ui::FlattenTool* flattenTool;
  ui::terrain_tool* terrainTool;
  QDockWidget* _terrain;
  ui::shader_tool* shaderTool;
  ui::texturing_tool* texturingTool;

  UIExitWarning *escWarning;
  ui::water_save_warning *waterSaveWarning;
  UIHelperModels *HelperModels;

  explicit UIMapViewGUI ( MapView* setMapview
                        , noggit::camera*
                        , float* tablet_pressure
                        , bool* tablet_active
                        );
};
