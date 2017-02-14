// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/shader_tool.hpp>

#include <noggit/application.h>
#include <noggit/Environment.h>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Gradient.h>
#include <noggit/ui/Slider.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/ToggleGroup.h>


namespace ui
{
  shader_tool::shader_tool(float x, float y, bool tablet)
    : UIWindow(x, y, winWidth, tablet ? 200.0f : 160.0f)
    , _radius(15.0f)
    , _speed(1.0f)
    , _red(Environment::getInstance()->cursorColorR)
    , _green(Environment::getInstance()->cursorColorG)
    , _blue(Environment::getInstance()->cursorColorB)
    , _tablet(tablet)
    , _tablet_control(eTabletControl_On)
  {
    addChild(new UIText(78.5f, 2.0f, "Shader", app.getArial14(), eJustifyCenter));

    _radius_slider = new UISlider(6.0f, 33.0f, 167.0f, 1000.0f, 0.00001f);
    _radius_slider->setFunc([&](float f) { _radius = f; });
    _radius_slider->setValue(_radius / 1000.0f);
    _radius_slider->setText("Radius: ");
    addChild(_radius_slider);

    _speed_slider = new UISlider(6.0f, 59.0f, 167.0f, 10.0f, 0.00001f);
    _speed_slider->setFunc([&](float f) { _speed = f; });
    _speed_slider->setValue(_speed / 10.0f);
    _speed_slider->setText("Speed: ");
    addChild(_speed_slider);

    addChild(new UIText(5.0f, 80.0f, "Color:", app.getArial12(), eJustifyLeft));

    _red_gradient = new UIGradient(6.0f, 100.0f, 167.0f, 15.0f, true);
    _red_gradient->setClickFunc([&](float f) { _red = f * 2.0f; });
    _red_gradient->setMinColor(1.0f, 0.0f, 0.0f, 0.0f);
    _red_gradient->setMaxColor(1.0f, 0.0f, 0.0f, 1.0f);
    _red_gradient->setClickColor(0.0f, 0.0f, 0.0f, 1.0f);
    _red_gradient->setValue(_red / 2.0f);
    addChild(_red_gradient);

    _green_gradient = new UIGradient(6.0f, 120.0f, 167.0f, 15.0f, true);
    _green_gradient->setClickFunc([&](float f) { _green = f * 2.0f; });
    _green_gradient->setMinColor(0.0f, 1.0f, 0.0f, 0.0f);
    _green_gradient->setMaxColor(0.0f, 1.0f, 0.0f, 1.0f);
    _green_gradient->setClickColor(0.0f, 0.0f, 0.0f, 1.0f);
    _green_gradient->setValue(_green / 2.0f);
    addChild(_green_gradient);

    _blue_gradient = new UIGradient(6.0f, 140.0f, 167.0f, 15.0f, true);
    _blue_gradient->setClickFunc([&](float f) { _blue = f * 2.0f; });
    _blue_gradient->setMinColor(0.0f, 0.0f, 1.0f, 0.0f);
    _blue_gradient->setMaxColor(0.0f, 0.0f, 1.0f, 1.0f);
    _blue_gradient->setClickColor(0.0f, 0.0f, 0.0f, 1.0f);
    _blue_gradient->setValue(_blue / 2.0f);
    addChild(_blue_gradient);

    if (tablet)
    {
      addChild(new UIText(78.5f, 137.0f, "Tablet Control", app.getArial14(), eJustifyCenter));

      _tablet_control_toggle = new UIToggleGroup(&_tablet_control);
      addChild(new UICheckBox(6.0f, 151.0f, "Off", _tablet_control_toggle, eTabletControl_Off));
      addChild(new UICheckBox(85.0f, 151.0f, "On", _tablet_control_toggle, eTabletControl_On));
      _tablet_control_toggle->Activate(eTabletControl_Off);
    }
  }

  void shader_tool::changeShader(math::vector_3d const& pos, float dt, bool add)
  {
    gWorld->changeShader (pos, 2.0f*dt*_speed, _radius, add);
  }

  void shader_tool::changeRadius(float change)
  {
    _radius = std::max(0.0f, std::min(1000.0f, _radius + change));
    _radius_slider->setValue(_radius / 1000.0f);
  }

  void shader_tool::changeSpeed(float change)
  {
    _speed = std::max(0.0f, std::min(10.0f, _speed + change));
    _speed_slider->setValue(_speed / 10.0f);
  }

  void shader_tool::setTabletControlValue(float pressure)
  {
    if (_tablet_control == eTabletControl_On)
    {
      _radius = std::max(0.0f, std::min(1000.0f, pressure / 20.48f));
      _radius_slider->setValue(_radius / 1000.0f);
    }
  }
}

