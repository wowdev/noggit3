#ifndef __GUI_H
#define __GUI_H

class UIToolbar;
class UIStatusBar;
class UIDetailInfos;
class UIDoodadSpawner;
class UIAppInfo;
class UIMinimapWindow;
class UIZoneIDBrowser;
class MapView;
class UIFrame;
class UITexturePicker;
class UITextureSwitcher;
class UIHelp;
class UICursorSwitcher;

#include "UIFrame.h"

//! \todo Give better name.
class UIMapViewGUI : public UIFrame
{
private:
  bool _tilemode;
  UICursorSwitcher* CursorSwitcher;
  UIHelp* _help;
  UIDoodadSpawner* _test;

public:
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
  UIDetailInfos* guidetailInfos;
  UIAppInfo* guiappInfo;
  UIZoneIDBrowser* ZoneIDBrowser;
  UITexturePicker* TexturePicker;
  UITextureSwitcher* TextureSwitcher;

  explicit UIMapViewGUI( MapView* setMapview );

  void setTilemode( bool enabled );
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

#endif
