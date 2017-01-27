// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>

class UIToggleGroup;
class UISlider;

namespace ui
{
  class terrain_tool : public UIWindow
  {
  public:
    terrain_tool(float x, float y, bool tablet);

    void changeTerrain(float dt);
    void moveVertices(float dt);
    void flattenVertices();

    void nextType();
    void changeRadius(float change);
    void changeSpeed(float change);
    void changeOrientation(float change);
    void changeAngle(float change);

    void setOrientation(float orientation);

    void setTabletControlValue(float pressure);

    float brushRadius() const { return _radius; }
  
  private:
    static const int winWidth = 180;
    
    float _radius;
    float _speed;
    float _angle;
    float _orientation;

    bool _tablet;

    int& _edit_type;
    int _vertex_mode;
    int _tablet_control;
    int _tablet_active_group;
  
    // UI stuff:

    UIToggleGroup* _type_toggle;
    UIToggleGroup* _vertex_toggle;
    UIToggleGroup* _tablet_control_toggle = nullptr;
    UIToggleGroup* _tablet_active_group_toggle = nullptr;

    UISlider* _radius_slider;
    UISlider* _speed_slider;
  };
}

