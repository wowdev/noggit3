// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/shader_tool.hpp>

#include <noggit/application.h>
#include <noggit/Environment.h>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/ui/CheckBox.h>
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

    _red_slider = new UISlider(6.0f, 85.0f, 167.0f, 2.0f, 0.00001f);
    _red_slider->setFunc([&](float f) { _red = f; });
    _red_slider->setValue(_red / 2.0f);
    _red_slider->setText("Red: ");
    addChild(_red_slider);

    _green_slider = new UISlider(6.0f, 111.0f, 167.0f, 2.0f, 0.00001f);
    _green_slider->setFunc([&](float f) { _green = f; });
    _green_slider->setValue(_green / 2.0f);
    _green_slider->setText("Green: ");
    addChild(_green_slider);

    _blue_slider = new UISlider(6.0f, 137.0f, 167.0f, 2.0f, 0.00001f);
    _blue_slider->setFunc([&](float f) { _blue = f; });
    _blue_slider->setValue(_blue / 2.0f);
    _blue_slider->setText("Blue: ");
    addChild(_blue_slider);

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

