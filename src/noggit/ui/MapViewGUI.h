// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class UIToolbar;
class UIStatusBar;
class UIMinimapWindow;
class MapView;
class UIFrame;
class UITexturePicker;
class UITextureSwitcher;
class UIHelp;
class UICursorSwitcher;
class UIWindow;
class UIExitWarning;
class UIHelperModels;
class UIWater;
class UIObjectEditor;
class UIRotationEditor;
namespace ui
{
  class caps_warning;
  class detail_infos;
  class FlattenTool;
  class shader_tool;
  class terrain_tool;
  class texturing_tool;
  class zone_id_browser;
  class water_save_warning;
  class water_type_browser;
}

#include <noggit/ui/Frame.h>
#include <noggit/ui/CurrentTexture.h>

//! \todo Give better name.
class UIMapViewGUI : public UIFrame
{
private:
  bool _tilemode;
  UICursorSwitcher* CursorSwitcher;
  UIHelp* _help;
  const math::vector_3d* _camera_pos;

public:
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
  UIMinimapWindow* minimapWindow;
  UIToolbar* guiToolbar;
  UIStatusBar* guiStatusbar;
  ui::detail_infos* guidetailInfos;
  ui::zone_id_browser* ZoneIDBrowser;
  UITexturePicker* TexturePicker;
  UIWater* guiWater;
  ui::water_type_browser* guiWaterTypeSelector;
  UICurrentTexture* guiCurrentTexture;
  UIObjectEditor* objectEditor;
  UIRotationEditor* rotationEditor;
  ui::FlattenTool* flattenTool;
  ui::terrain_tool* terrainTool;
  ui::shader_tool* shaderTool;
  ui::texturing_tool* texturingTool;

  UIExitWarning *escWarning;
  ui::caps_warning *capsWarning;
  ui::water_save_warning *waterSaveWarning;
  UIHelperModels *HelperModels;

  explicit UIMapViewGUI(MapView* setMapview, const math::vector_3d* camera_pos);

  void setTilemode(bool enabled);
  virtual void render() const;

  void showCursorSwitcher();
  void hideCursorSwitcher();
  void toggleCursorSwitcher();
  void showHelp();
  void hideHelp();
  void toggleHelp();

  void showTest();
  void hideTest();
  void toggleTest();
};
