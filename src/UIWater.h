#ifndef __UI_WATER_H
#define __UI_WATER_H

#include "UICloseWindow.h"
#include "MapView.h"
class UISlider;
class UIText;
class UIButton;

class UIWater : public UIWindow
{
public:
  UIWater(MapView *mapView );

  void resize();

  void updatePos(int newTileX, int newTileY);
  void updateData();

  void setWaterTrans(float val);
  void setWaterHeight(float val);
  void changeWaterHeight(UIFrame::Ptr,int someint);
  void addWaterLayer(UIFrame::Ptr, int);
  void deleteWaterLayer(UIFrame::Ptr ptr, int someint);
  void changeWaterType(UIFrame::Ptr /*ptr*/, int someint);


private:
  static const int winWidth = 180;
  static const int winHeight = 200;

  MapView *_MapView;
  UISlider *waterOpercity;
  UIText *waterLevel;
  UIButton *waterType;

  int tileX;
  int tileY;
};

#endif
