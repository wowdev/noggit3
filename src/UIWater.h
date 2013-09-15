#ifndef __UI_WATER_H
#define __UI_WATER_H

#include "UICloseWindow.h"
#include "MapView.h"
class UISlider;

class UIWater : public UIWindow
{
private:
  static const int winWidth = 180;
  static const int winHeight = 200;
  MapView *_MapView;
  UISlider *waterOpercity;
  UISlider *waterLevel;
public:
  UIWater(MapView *mapView );
  void resize();
  void setWaterOpercity(float level);
};

#endif