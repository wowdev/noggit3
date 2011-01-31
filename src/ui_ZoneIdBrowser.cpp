#include <iostream>
#include <sstream>

#include "ui_ZoneIdBrowser.h"
#include "ui_ListView.h"
#include "Gui.h"
#include "scrollbarUI.h"
#include "textUI.h" // textUI
#include "noggit.h" // arial14, arialn13
#include "dbc.h"

ui_ZoneIdBrowser::ui_ZoneIdBrowser(int xPos,int yPos, int w, int h, Gui *setGui) : window(xPos,yPos,w,h)
{
	this->mainGui = setGui;
	this->ZoneIdList = new ui_ListView(4,4,w - 8,h - 8,20);
	this->ZoneIdList->clickable = true;
	

	for( DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i ) 
	{
		frame *curFrame = new frame(1,1,1,1); 
		std::stringstream ss;
		ss << i->getInt(AreaDB::AreaID) << "-" << gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
		curFrame->addChild(new textUI( 0.0f, 5.0f, ss.str() , &arialn13, eJustifyLeft ) );
		this->ZoneIdList->addElement(curFrame);
	}

	this->addChild(ZoneIdList);
}
