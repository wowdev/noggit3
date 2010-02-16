#ifndef __GUI_H
#define __GUI_H

class Gui;

#include "Toolbar.h"
#include "statusBar.h"
#include "detailInfos.h"
#include "appInfo.h"

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