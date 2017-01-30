// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>
#include <noggit/tile_index.hpp>

class UISlider;
class UIText;
class UIButton;
class UICheckBox;
class UIMapViewGUI;

class UIWater : public UIWindow
{
public:
  UIWater(UIMapViewGUI *setGui);

  void updatePos(tile_index const& newTile);
  void updateData();

  void setWaterTrans(float val);
  void changeWaterHeight(float);
  void changeWaterType(int waterint);

private:
  static const int winWidth = 180;
  static const int winHeight = 350;

  UIMapViewGUI *mainGui;
  UISlider *waterOpacity;
  UIText *waterLevel;
  UIButton *waterType;
  UIButton *waterGen;
  UIButton *addWater;
  UIButton *cropWater;
  UISlider *waterGenFactor;
  UIText *waterLayer;

  tile_index tile;
};
