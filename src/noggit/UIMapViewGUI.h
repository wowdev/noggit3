#ifndef __GUI_H
#define __GUI_H

class UIToolbar;
class UIStatusBar;
class UIDetailInfos;
class UIAppInfo;
class minimap_widget;
class UIZoneIDBrowser;
class MapView;
class UIFrame;
class UITexturePicker;
class UITextureSwitcher;
class UICursorSwitcher;
class World;

#include <noggit/UIFrame.h>

//! \todo Give better name.
class UIMapViewGUI : public UIFrame
{
public:
  explicit UIMapViewGUI( World* world, MapView* setMapview );
  // Editor paramter
  int ground_edit_mode;
  int selection_view_mode;

  MapView* theMapview;
  // UI elements
  UIFrame* TexturePalette;
  UIFrame* SelectedTexture;
  UIToolbar* guiToolbar;
  UIStatusBar* guiStatusbar;
  UIDetailInfos* guidetailInfos;
  UIAppInfo* guiappInfo;
  UIZoneIDBrowser* ZoneIDBrowser;
  UITexturePicker* TexturePicker;
  UITextureSwitcher* TextureSwitcher;

  void setTilemode( bool enabled );
  virtual void render() const;

  void showCursorSwitcher();
  void hideCursorSwitcher();
  void toggleCursorSwitcher();

private:
  bool _tilemode;
  UICursorSwitcher* CursorSwitcher;
  World* _world;
};

#endif
