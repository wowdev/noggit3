#ifndef __ZONEIDBROWSER_H
#define __ZONEIDBROWSER_H

#include "window.h"
class Gui;
class ui_ListView;

class ui_ZoneIdBrowser : public window
{
public:
	ui_ZoneIdBrowser(int xPos,int yPos, int w, int h, Gui *setGui);
	void setMapID(int id);

private:
		Gui *mainGui;
		ui_ListView *ZoneIdList;
		int mapID;
		void buildAreaList();
};

#endif