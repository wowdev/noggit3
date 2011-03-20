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
	minimapWindowUI *minimapWindow;
	Toolbar *guiToolbar;
	statusBar *guiStatusbar;
	detailInfos *guidetailInfos;
	appInfo *guiappInfo;
	ui_ZoneIdBrowser *ZoneIDBrowser;

	Gui(MapView *setMapview);
	~Gui();
	
	void render(bool tilemode);
	void resize();
};

#endif
