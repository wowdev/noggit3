// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>

class UISlider;
class UIToggleGroup;

namespace ui
{
  class shader_tool : public UIWindow
  {
  public:
    shader_tool(float x, float y, bool tablet);

    void changeShader(float dt, bool add);

    void changeRadius(float change);
    void changeSpeed(float change);

    void setTabletControlValue(float pressure);

    float brushRadius() const { return _radius; }
  
  private:
    static const int winWidth = 180;
    
    float _radius;
    float _speed;
    float& _red;
    float& _green;
    float& _blue;

    bool _tablet;

    int _tablet_control;
  
    // UI stuff:
    UIToggleGroup* _tablet_control_toggle = nullptr;

    UISlider* _radius_slider;
    UISlider* _speed_slider;
    UISlider* _red_slider;
    UISlider* _green_slider;
    UISlider* _blue_slider;
  };
}

