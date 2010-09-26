#ifndef __GUI_H
#define __GUI_H

class Toolbar;
class statusBar;
class detailInfos;
class appInfo;

class Gui
{
public:
	// Editor paramter
	int ground_edit_mode;
	int selection_view_mode;


	// UI elements
	Toolbar *guiToolbar;
	statusBar *guiStatusbar;
	detailInfos *guidetailInfos;
	appInfo *guiappInfo;

	Gui();
};
#endif
