#include "ui_ZoneIdBrowser.h"
#include "Gui.h"

ui_ZoneIdBrowser::ui_ZoneIdBrowser(int xPos,int yPos, int w, int h, Gui *setGui) : window(xPos,yPos,w,h)
{
	this->mainGui = setGui;
}
