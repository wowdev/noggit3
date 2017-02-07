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

  void changeWaterType(int waterint);

  void paintLiquid(math::vector_3d const& pos, bool add);

  void changeRadius(float change);
  float brushRadius() const { return _radius; }

private:
  static const int winWidth = 180;
  static const int winHeight = 350;

  int _liquid_id;
  float _radius;

  UIMapViewGUI *mainGui;
  UISlider* _radius_slider;

  UIButton *waterType;
  UIButton *cropWater;
  UIText *waterLayer;

  tile_index tile;
};
