#ifndef __UI_CAPSWARNING_H
#define __UI_CAPSWARNING_H
 
#include "UIWindow.h"
#include "MapView.h"

class UICapsWarning : public UIWindow
{
private:
  static const int winWidth = 320;
  static const int winHeight = 80;
  MapView *_MapView;
public:
  UICapsWarning(MapView *mapView );
  void resize();
};


#endif