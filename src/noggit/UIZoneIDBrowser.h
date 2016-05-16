// UIZoneIDBrowser.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Stephan Biegel <project.modcraft@googlemail.com>

#pragma once

#include <string>

#include <noggit/UIButton.h>
#include <noggit/UIWindow.h>
#include <noggit/UICloseWindow.h>

class UIMapViewGUI;
class UIListView;

class UIZoneIDBrowser : public UICloseWindow
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
  UIButton* closeBrowser;
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
