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
	this->MapName = new buttonUI( 4.0f, 4.0f, 160.0f, 28.0f, "no map selected", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", theButtonMapPressed, 0 );
	this->addChild(this->MapName);
	this->ZoneName = new buttonUI( 174.0f, 4.0f, 200.0f, 28.0f, "no zone selected", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", theButtonMapPressed, 1 );
	this->addChild(this->ZoneName);
	this->SubZoneName = new buttonUI( 384.0f, 4.0f, 310.0f, 28.0f, "no sub zone selected", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", theButtonMapPressed, 2 );
	this->addChild(this->SubZoneName);
}
 
void ui_ZoneIdBrowser::setMapID( int id )
{
	this->mapID = id;
	this->zoneID = 0;
	this->subZoneID = 0;
	for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i ) 
	{
		if( i->getInt( MapDB::MapID ) == id)
			this->MapName->setText( i->getString( MapDB::InternalName ) );
	}
	this->buildAreaList();
}

void ui_ZoneIdBrowser::setZoneID( int id )
{
	for( DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i ) 
	{
		if(i->getInt(AreaDB::AreaID) == id)
		{
			if(i->getUInt( AreaDB::Region ) == 0)
			{
				this->ZoneName->setText(gAreaDB.getAreaName(i->getInt(AreaDB::AreaID)));
				this->zoneID = id;
				this->subZoneID = 0;
				this->SubZoneName->setText("no sub zone selected");
				if(changeFunc)
					changeFunc(this,id);
				//this->collapseList();
			}
			else
			{
				this->SubZoneName->setText(gAreaDB.getAreaName(i->getInt(AreaDB::AreaID)));
				this->subZoneID = id;
				if(changeFunc)
					changeFunc(this,id);
				//this->collapseList();
			}
		}
	}
	buildAreaList();
}

void ui_ZoneIdBrowser::ButtonMapPressed( int id )
{
	if(id==1)
	{
		if(this->zoneID != 0)
		{
			this->SubZoneName->setText("no sub zone selected");
			this->subZoneID = 0;
			this->ZoneName->setText("no zone selected");
			this->zoneID = 0;
			if(changeFunc)
				changeFunc(this,0);
			buildAreaList();
			//this->expandList();
		}
	} 
	else if(id==2)
	{
		if(this->subZoneID != 0)
		{
				this->SubZoneName->setText("no sub zone selected");
				this->subZoneID = 0;
				if(changeFunc)
					changeFunc(this,this->zoneID);
			//this->expandList();
		}
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
						buttonUI *tempButton = new buttonUI(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", changeZoneValue, i->getInt(AreaDB::AreaID) );
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
						buttonUI *tempButton = new buttonUI(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", changeZoneValue, i->getInt(AreaDB::AreaID) );
						tempButton->setLeft();
						curFrame->addChild(tempButton);
						this->ZoneIdList->addElement(curFrame);
					}
				}
			}

		}
		this->ZoneIdList->recalcElements(1);
}

void ui_ZoneIdBrowser::setChangeFunc( void (*f)( frame *, int ))
{
	changeFunc=f;
}

void ui_ZoneIdBrowser::expandList()
{
	//this->height = this->heightExpanded;
	this->ZoneIdList->hidden = false;
}


void ui_ZoneIdBrowser::collapseList()
{
	//this->heightExpanded = this->height;
	//this->height=30;
	this->ZoneIdList->hidden = true;
}
