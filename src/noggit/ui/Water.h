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
  void changeOrientation(float change);
  void changeAngle(float change);
  void change_height(float change) { _lock_pos.y += change; }

  void lockPos(math::vector_3d const& cursor_pos);
  void toggle_lock();
  void toggle_angled_mode();

  float brushRadius() const { return _radius; }
  float angle() const { return _angle; }
  float orientation() const { return _orientation; }
  bool angled_mode() const { return _angled_mode; }
  bool use_ref_pos() const { return _locked; }
  math::vector_3d ref_pos() const { return _lock_pos; }

private:
  float get_opacity_factor() const;

  static const int winWidth = 180;
  static const int winHeight = 450;

  int _liquid_id;
  float _radius;

  float _angle;
  float _orientation;

  bool _locked;
  bool _angled_mode;

  bool _override_liquid_id;
  bool _override_height;

  int _opacity_mode;
  float _custom_opacity_factor;

  math::vector_3d _lock_pos;  

  UIMapViewGUI *mainGui;
  UISlider* _radius_slider;
  UISlider* _angle_slider;
  UISlider* _orientation_slider;

  UIText* _lock_display;

  UICheckBox* _angle_checkbox;
  UICheckBox* _lock_checkbox;

  UIButton *waterType;
  UIButton *cropWater;
  UIText *waterLayer;

  tile_index tile;
};

