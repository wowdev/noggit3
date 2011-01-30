#include "ui_ZoneIdBrowser.h"
#include "ui_ListView.h"
#include "Gui.h"
#include "scrollbarUI.h"


ui_ZoneIdBrowser::ui_ZoneIdBrowser(int xPos,int yPos, int w, int h, Gui *setGui) : window(xPos,yPos,w,h)
{
	this->mainGui = setGui;
	this->ZoneIdList = new ui_ListView(4,20,w - 8,h - 26);
	this->ZoneIdList->clickable = true;
	this->addChild(ZoneIdList);
}
