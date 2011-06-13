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
  UIZoneIDBrowser(int xPos,int yPos, int w, int h, UIMapViewGUI *setGui);
  void setMapID(int id);
  void setZoneID( int id );
  void ButtonMapPressed( int id );
  void refreshMapPath();
  void setChangeFunc( void (*f)( UIFrame *, int ));
private:
  void ( *changeFunc )( UIFrame *, int );
  UIMapViewGUI *mainGui;
  UIListView* ZoneIdList;
  int heightExpanded;
  int mapID;
  unsigned int zoneID;
  int subZoneID;
  int selectedAreaID;
  void buildAreaList();
  void expandList();
  void collapseList();
  std::string MapName;
  std::string ZoneName;
  std::string SubZoneName;
  UIButton* backZone;
  UIText* ZoneIDPath;
};

#endif