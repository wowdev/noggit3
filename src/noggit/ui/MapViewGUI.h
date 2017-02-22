// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class UIToolbar;
class UIStatusBar;
class UIDetailInfos;
class UIMinimapWindow;
class MapView;
class UIFrame;
class UITexturePicker;
class UITextureSwitcher;
class UIHelp;
class UICursorSwitcher;
class UIWindow;
class UIExitWarning;
class UICapsWarning;
class UIWaterSaveWarning;
class UIHelperModels;
class UISlider;
class UIGradient;
class UIWater;
class UIObjectEditor;
class UIRotationEditor;
namespace ui
{
  class FlattenTool;
  class shader_tool;
  class terrain_tool;
  class texturing_tool;
  class zone_id_browser;
}

class UIModel;
class UIAlphamap;
class UIWaterTypeBrowser;

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
  UIModel* ModelBrowser;
  UIFrame* SelectedTexture;
  UIMinimapWindow* minimapWindow;
  UIToolbar* guiToolbar;
  UIStatusBar* guiStatusbar;
  UIDetailInfos* guidetailInfos;
  ui::zone_id_browser* ZoneIDBrowser;
  UITexturePicker* TexturePicker;
  UIWater* guiWater;
  UIWaterTypeBrowser* guiWaterTypeSelector;
  UICurrentTexture* guiCurrentTexture;
  UIObjectEditor* objectEditor;
  UIRotationEditor* rotationEditor;
  ui::FlattenTool* flattenTool;
  ui::terrain_tool* terrainTool;
  ui::shader_tool* shaderTool;
  ui::texturing_tool* texturingTool;

  UIExitWarning *escWarning;
  UICapsWarning *capsWarning;
  UIWaterSaveWarning *waterSaveWarning;
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
