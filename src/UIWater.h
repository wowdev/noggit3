#ifndef __UI_WATER_H
#define __UI_WATER_H

#include "UICloseWindow.h"
#include "MapView.h"
class UISlider;

class UIWater : public UIWindow
{
public:
  UIWater(MapView *mapView );

  void resize();

  void updatePos(int newTileX, int newTileY);
  void updateData();

  void setWaterTrans(float val);
  void setWaterHeight(float val);
  void addWaterLayer(UIFrame::Ptr, int);
  void deleteWaterLayer(UIFrame::Ptr ptr, int someint);

private:
  static const int winWidth = 180;
  static const int winHeight = 200;

  MapView *_MapView;
  UISlider *waterOpercity;
  UISlider *waterLevel;

  int tileX;
  int tileY;
};

#endif
