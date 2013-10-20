#ifndef __MODELFROMTXT_H
#define __MODELFROMTXT_H

#include <string>

#include "UIButton.h"
#include "UIWindow.h"
#include "UICloseWindow.h"

class UIMapViewGUI;
class UIListView;

class UIModelSelectorFromTxt : public UICloseWindow
{
public:
  typedef UIModelSelectorFromTxt* Ptr;

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

  void buildModelList();
  void expandList();
  void collapseList();

public:
  UIModelSelectorFromTxt(int xPos,int yPos, int w, int h, UIMapViewGUI *setGui);
  void setMapID(int id);
  void setZoneID( int id );
  void ButtonMapPressed( int id );
  void refreshMapPath();
  void setChangeFunc( void (*f)( UIFrame *, int ));
};

#endif
