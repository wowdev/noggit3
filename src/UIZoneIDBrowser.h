#ifndef __ZONEIDBROWSER_H
#define __ZONEIDBROWSER_H

#include <string>

#include "UIButton.h"
#include "UIWindow.h"

class UIMapViewGUI;
class UIListView;

class UIZoneIDBrowser : public UIWindow
{
public:
  typedef UIZoneIDBrowser* Ptr;

private:
  void ( *changeFunc )( UIFrame *, int );
  UIMapViewGUI *mainGui;
  UIListView* ZoneIdList;
  int mapID;
  unsigned int zoneID;
  int subZoneID;
  int selectedAreaID;
  std::string MapName;
  std::string ZoneName;
  std::string SubZoneName;
  UIButton* backZone;
  UIText* ZoneIDPath;
  
  void buildAreaList();
  void expandList();
  void collapseList();
  
public:
  UIZoneIDBrowser(int xPos,int yPos, int w, int h, UIMapViewGUI *setGui);
  void setMapID(int id);
  void setZoneID( int id );
  void ButtonMapPressed( int id );
  void refreshMapPath();
  void setChangeFunc( void (*f)( UIFrame *, int ));
};

#endif