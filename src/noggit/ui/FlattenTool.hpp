// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/Window.h>


class UIToggleGroup;
class UISlider;
class UITextBox;
class UICheckBox;

namespace ui
{
  class FlattenTool : public UIWindow
  {  
  public:
    FlattenTool(float x, float y);

    void flatten(math::vector_3d const& cursor_pos, float dt);
    void blur(math::vector_3d const& cursor_pos, float dt);

    void nextFlattenType();
    void nextFlattenMode();
    void toggleFlattenAngle();
    void toggleFlattenLock();
    void lockPos (math::vector_3d const& cursor_pos);

    void changeRadius(float change);
    void changeSpeed(float change);
    void changeOrientation(float change);
    void changeAngle(float change);
    void changeHeight(float change);

    void setRadius(float radius);

    float brushRadius() const { return _radius; }
    float angle() const { return _angle; }
    float orientation() const { return _orientation; }
    math::vector_3d ref_pos() const { return _lock_pos; }

  private:
    static const int winWidth = 180;
    static const int winHeight = 400;

    float _radius;
    float _speed;
    float& _angle;
    float& _orientation;

    math::vector_3d _lock_pos;

    bool _locked;
    bool& _angled_mode;

    int _flatten_type;
    int _flatten_mode;

    // UI stuff
  public:
    void updateLockCoord(UITextBox* cb, std::string const& str, float& value);

  private:
    UIToggleGroup* _type_toggle;
    UIToggleGroup* _mode_toggle;

    UISlider* _radius_slider;
    UISlider* _speed_slider;
    UISlider* _angle_slider;
    UISlider* _orientation_slider;

    UITextBox* _lock_x;
    UITextBox* _lock_z;
    UITextBox* _lock_h;

    UICheckBox* _angle_checkbox;
    UICheckBox* _lock_checkbox;
  };
}

