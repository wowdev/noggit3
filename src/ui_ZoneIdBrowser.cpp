#include <iostream>
#include <sstream>

#include "ui_ZoneIdBrowser.h"
#include "ui_ListView.h"
#include "Gui.h"
#include "scrollbarUI.h"
#include "textUI.h" // textUI
#include "noggit.h" // arial14, arialn13
#include "dbc.h"
#include "log.h"
#include "buttonUI.h"
#include "dbc.h"
#include "misc.h"

void theButtonMapPressed(frame *f,int id)
{
	((ui_ZoneIdBrowser *)(f->parent))->ButtonMapPressed(id);
}

void changeZoneValue(frame *f,int id)
{
	((ui_ZoneIdBrowser *)(f->parent->parent->parent))->setZoneID(id);
}

ui_ZoneIdBrowser::ui_ZoneIdBrowser(int xPos,int yPos, int w, int h, Gui *setGui) : window(xPos,yPos,w,h)
{
	changeFunc=0;
	this->mainGui = setGui;
	this->ZoneIDPath = new textUI( 10.0f, 6.0f, "TEST", arial12, eJustifyLeft);
	this->addChild(this->ZoneIDPath);

	this->backZone = new buttonUI( 407.0f, 2.0f, 24.0f, 24.0f, "", "Interface\\BUTTONS\\UI-RotationLeft-Button-Up.blp", "Interface\\BUTTONS\\UI-RotationLeft-Button-Down.blp", theButtonMapPressed, 0 );
	this->addChild(this->backZone);

}
 
void ui_ZoneIdBrowser::setMapID( int id )
{
	this->mapID = id;
	this->zoneID = 0;
	this->subZoneID = 0;
	for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i ) 
	{
		if( i->getInt( MapDB::MapID ) == id)
			this->MapName = i->getString( MapDB::InternalName );
	}
	this->buildAreaList();
	this->refreshMapPath();
}

void ui_ZoneIdBrowser::setZoneID( int id )
{
	for( DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i ) 
	{
		if(i->getInt(AreaDB::AreaID) == id)
		{
			if(i->getUInt( AreaDB::Region ) == 0)
			{
				this->ZoneName = gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
				this->zoneID = id;
				this->subZoneID = 0;
				this->SubZoneName = "";;
				if(changeFunc)
					changeFunc(this,id);
			}
			else
			{
				this->SubZoneName = gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
				this->subZoneID = id;
				if(changeFunc)
					changeFunc(this,id);
			}
		}
	}
	this->buildAreaList();
	this->refreshMapPath();
}

void ui_ZoneIdBrowser::ButtonMapPressed( int id )
{
	if(id==0)
	{
		if(this->subZoneID)
		{
			// clear subzone
			this->subZoneID=0;
			this->SubZoneName="";
			if(changeFunc)
				changeFunc(this,this->zoneID);
		}
		else
		{
			// clear zone
			this->zoneID=0;
			this->ZoneName="";
			if(changeFunc)
				changeFunc(this,0);
		}
		this->refreshMapPath();
		buildAreaList();
	}
}

void ui_ZoneIdBrowser::buildAreaList()
{
	this->removeChild(this->ZoneIdList);
	this->ZoneIdList = NULL;
	this->ZoneIdList = new ui_ListView(4,24,this->width - 8,this->height - 28,20);
	this->ZoneIdList->clickable = true;
	this->addChild(ZoneIdList);
		//  Read out Area List.
		for( DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i ) 
		{
			if( i->getInt(AreaDB::Continent) == this->mapID )
			{
				if(	this->zoneID == 0)
				{
					if(i->getUInt( AreaDB::Region ) == 0)
					{
						frame *curFrame = new frame(1,1,1,1); 
						std::stringstream ss;
						ss << i->getInt(AreaDB::AreaID) << "-" << misc::replaceSpezialChars(gAreaDB.getAreaName(i->getInt(AreaDB::AreaID)));
						buttonUI *tempButton = new buttonUI(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", changeZoneValue, i->getInt(AreaDB::AreaID) );
						tempButton->setLeft();
						curFrame->addChild(tempButton);
						this->ZoneIdList->addElement(curFrame);
					}
				}
				else if(	this->zoneID > 0)
				{
					if(i->getUInt( AreaDB::Region ) == this->zoneID)
					{
						frame *curFrame = new frame(1,1,1,1); 
						std::stringstream ss;
						ss << i->getInt(AreaDB::AreaID) << "-" << misc::replaceSpezialChars(gAreaDB.getAreaName(i->getInt(AreaDB::AreaID)));
						buttonUI *tempButton = new buttonUI(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", changeZoneValue, i->getInt(AreaDB::AreaID) );
						tempButton->setLeft();
						curFrame->addChild(tempButton);
						this->ZoneIdList->addElement(curFrame);
					}
				}
			}

		}
		this->ZoneIdList->recalcElements(1);
}

void ui_ZoneIdBrowser::refreshMapPath()
{
	std::stringstream AreaPath;
	if(this->SubZoneName!="")
		AreaPath << this->MapName << " < " << this->SubZoneName;
	else
		AreaPath << this->MapName << " < " << this->ZoneName ;
	this->ZoneIDPath->setText( AreaPath.str() );
}

void ui_ZoneIdBrowser::setChangeFunc( void (*f)( frame *, int ))
{
	changeFunc=f;
}
