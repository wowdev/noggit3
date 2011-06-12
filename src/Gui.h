#ifndef __GUI_H
#define __GUI_H

class Toolbar;
class statusBar;
class detailInfos;
class appInfo;
class minimapWindowUI;
class ui_ZoneIdBrowser;
class MapView;
class frame;
class uiTexturePicker;

//! \todo Give better name.
class Gui
{
public:
  // Editor paramter
  int ground_edit_mode;
  int selection_view_mode;
  frame* tileFrames;

  MapView *theMapview;
  // UI elements
  frame  *TexturePalette;
  frame  *SelectedTexture;
  minimapWindowUI *minimapWindow;
  Toolbar *guiToolbar;
  statusBar *guiStatusbar;
  detailInfos *guidetailInfos;
  appInfo *guiappInfo;
  ui_ZoneIdBrowser *ZoneIDBrowser;
  uiTexturePicker *TexturePicker;
  explicit Gui(MapView *setMapview);
  ~Gui();
  
  void render(bool tilemode);
  void resize();
};

#endif
