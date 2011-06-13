#include "UIZoneIDBrowser.h"

#include <iostream>
#include <sstream>
#include <string>

#include "DBC.h"
#include "Log.h"
#include "Misc.h"
#include "Noggit.h" // arial14, arialn13
#include "UIButton.h"
#include "UIListView.h"
#include "UIMapViewGUI.h"
#include "UIScrollBar.h"
#include "UIText.h" // UIText

void theButtonMapPressed(UIFrame *f,int id)
{
  (reinterpret_cast<UIZoneIDBrowser*>(f->parent))->ButtonMapPressed(id);
}

void changeZoneValue(UIFrame *f,int id)
{
  (reinterpret_cast<UIZoneIDBrowser*>(f->parent->parent->parent))->setZoneID(id);
}

UIZoneIDBrowser::UIZoneIDBrowser(int xPos,int yPos, int w, int h, UIMapViewGUI *setGui)
: UIWindow(xPos,yPos,w,h)
, changeFunc( NULL )
, mainGui( setGui )
, ZoneIdList( NULL )
, mapID( -1 )
, zoneID( -1 )
, subZoneID( -1 )
, selectedAreaID( -1 )
, MapName( "" )
, ZoneName( "" )
, SubZoneName( "" )
, backZone( new UIButton( 407.0f, 2.0f, 24.0f, 24.0f, "", "Interface\\BUTTONS\\UI-RotationLeft-Button-Up.blp", "Interface\\BUTTONS\\UI-RotationLeft-Button-Down.blp", theButtonMapPressed, 0 ) )
, ZoneIDPath( new UIText( 10.0f, 6.0f, "", arial12, eJustifyLeft) )
{
  this->addChild(this->ZoneIDPath);
  this->addChild(this->backZone);
}
 
void UIZoneIDBrowser::setMapID( int id )
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

void UIZoneIDBrowser::setZoneID( int id )
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

void UIZoneIDBrowser::ButtonMapPressed( int id )
{
  if( id == 0 )
  {
    if(this->subZoneID)
    {
      // clear subzone
      this->subZoneID = 0;
      this->SubZoneName = "";
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

void UIZoneIDBrowser::buildAreaList()
{
  this->removeChild( this->ZoneIdList );
  this->ZoneIdList = NULL;
  this->ZoneIdList = new UIListView(4,24,this->width - 8,this->height - 28,20);
  this->ZoneIdList->clickable = true;
  this->addChild(ZoneIdList);
    //  Read out Area List.
    for( DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i ) 
    {
      if( i->getInt(AreaDB::Continent) == this->mapID )
      {
        if(  this->zoneID == 0)
        {
          if(i->getUInt( AreaDB::Region ) == 0)
          {
            UIFrame *curFrame = new UIFrame(1,1,1,1); 
            std::stringstream ss;
            ss << i->getInt(AreaDB::AreaID) << "-" << gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
            UIButton *tempButton = new UIButton(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", changeZoneValue, i->getInt(AreaDB::AreaID) );
            tempButton->setLeft();
            curFrame->addChild(tempButton);
            this->ZoneIdList->addElement(curFrame);
          }
        }
        else if(  this->zoneID > 0)
        {
          if(i->getUInt( AreaDB::Region ) == this->zoneID)
          {
            UIFrame *curFrame = new UIFrame(1,1,1,1); 
            std::stringstream ss;
            ss << i->getInt(AreaDB::AreaID) << "-" << gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
            UIButton *tempButton = new UIButton(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", changeZoneValue, i->getInt(AreaDB::AreaID) );
            tempButton->setLeft();
            curFrame->addChild(tempButton);
            this->ZoneIdList->addElement(curFrame);
          }
        }
      }

    }
    this->ZoneIdList->recalcElements(1);
}

void UIZoneIDBrowser::refreshMapPath()
{
  std::stringstream AreaPath;
  if(this->SubZoneName!="")
    AreaPath << this->MapName << " < " << this->SubZoneName;
  else
    AreaPath << this->MapName << " < " << this->ZoneName ;
  this->ZoneIDPath->setText( AreaPath.str() );
}

void UIZoneIDBrowser::setChangeFunc( void (*f)( UIFrame *, int ))
{
  changeFunc=f;
}
