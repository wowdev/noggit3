#ifndef __ZONEIDBROWSER_H
#define __ZONEIDBROWSER_H

#include "window.h"
#include "Gui.h"

class ui_ZoneIdBrowser : public window
{
public:
	ui_ZoneIdBrowser(int xPos,int yPos, int w, int h, Gui *setGui);
private:
		Gui *mainGui;
};

#endif