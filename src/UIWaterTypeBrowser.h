#ifndef __WATERIDBROWSER_H
#define __WATERIDBROWSER_H

#include <string>

#include "UIButton.h"
#include "UIWindow.h"
#include "UICloseWindow.h"

class UIMapViewGUI;
class UIListView;

class UIWaterTypeBrowser : public UICloseWindow
{
public:
  typedef UIWaterTypeBrowser* Ptr;

private:
  void ( *changeFunc )( UIFrame *, int );
  UIMapViewGUI *mainGui;
  UIListView* WaterTypeList;
  


public:
  UIWaterTypeBrowser(int xPos,int yPos, int w, int h, UIMapViewGUI *setGui);
  void setChangeFunc( void (*f)( UIFrame *, int ));
  void setWaterTypeID( int id );
  void buildTypeList();
};

#endif
