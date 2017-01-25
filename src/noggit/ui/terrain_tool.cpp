// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/terrain_tool.hpp>

#include <noggit/application.h>
#include <noggit/Environment.h>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Slider.h>
#include <noggit/ui/ToggleGroup.h>
#include <noggit/ui/Text.h>


namespace ui
{
  terrain_tool::terrain_tool(float x, float y, bool tablet)
    : UIWindow(x, y, winWidth, tablet ? 240.0f : 160.0f)
    , _radius(15.0f)
    , _speed(2.0f)
    , _tablet(tablet)
    , _edit_type(Environment::getInstance()->groundBrushType)
    , _tablet_control(eTabletControl_On)
    , _tablet_active_group(eTerrainTabletActiveGroup_Speed)
  {
    addChild(new UIText(78.5f, 2.0f, "Raise / Lower", app.getArial14(), eJustifyCenter));

    _type_toggle = new UIToggleGroup(&_edit_type);
    addChild(new UICheckBox(6.0f, 15.0f, "Flat", _type_toggle, eTerrainType_Flat));
    addChild(new UICheckBox(85.0f, 15.0f, "Linear", _type_toggle, eTerrainType_Linear));
    addChild(new UICheckBox(6.0f, 40.0f, "Smooth", _type_toggle, eTerrainType_Smooth));
    addChild(new UICheckBox(85.0f, 40.0f, "Polynomial", _type_toggle, eTerrainType_Polynom));
    addChild(new UICheckBox(6.0f, 65.0f, "Trigonom", _type_toggle, eTerrainType_Trigo));
    addChild(new UICheckBox(85.0f, 65.0f, "Quadratic", _type_toggle, eTerrainType_Quadra));
    _type_toggle->Activate(eTerrainType_Linear);

    _radius_slider = new UISlider(6.0f, 120.0f, 167.0f, 1000.0f, 0.00001f);
    _radius_slider->setFunc([&](float f) { _radius = f; });
    _radius_slider->setValue(_radius / 1000);
    _radius_slider->setText("Brush radius: ");
    addChild(_radius_slider);

    _speed_slider = new UISlider(6.0f, 145.0f, 167.0f, 10.0f, 0.00001f);
    _speed_slider->setFunc([&](float f) { _speed = f; });
    _speed_slider->setValue(_speed / 10.0f);
    _speed_slider->setText("Brush Speed: ");
    addChild(_speed_slider);

    if (tablet)
    {
      addChild(new UIText(78.5f, 170.0f, "Tablet Control", app.getArial14(), eJustifyCenter));

      _tablet_control_toggle = new UIToggleGroup(&_tablet_control);
      addChild(new UICheckBox(6.0f, 182.0f, "Off", _tablet_control_toggle, eTabletControl_Off));
      addChild(new UICheckBox(85.0f, 182.0f, "On", _tablet_control_toggle, eTabletControl_On));
      _tablet_control_toggle->Activate(eTabletControl_On);

      _tablet_active_group_toggle = new UIToggleGroup(&_tablet_active_group);
      addChild(new UICheckBox(6.0f, 207.0f, "Radius", _tablet_active_group_toggle, eTerrainTabletActiveGroup_Radius));
      addChild(new UICheckBox(85.0f, 207.0f, "Speed", _tablet_active_group_toggle, eTerrainTabletActiveGroup_Speed));
      _tablet_active_group_toggle->Activate(eTerrainTabletActiveGroup_Speed);
    }
  }

  void terrain_tool::changeTerrain(float dt)
  {
    gWorld->changeTerrain(Environment::getInstance()->Pos3DX, Environment::getInstance()->Pos3DZ, dt*_speed, _radius, _edit_type);
  }

  void terrain_tool::nextType()
  {
    _edit_type = (++_edit_type) % eTerrainType_Count;
    _type_toggle->Activate(_edit_type);
  }

  void terrain_tool::changeRadius(float change)
  {
    _radius = std::max(0.0f, std::min(1000.0f, _radius + change));
    _radius_slider->setValue(_radius / 1000.0f);
  }

  void terrain_tool::changeSpeed(float change)
  {
    _speed = std::max(0.0f, std::min(10.0f, _speed + change));
    _speed_slider->setValue(_speed / 10.0f);
  }

  void terrain_tool::setTabletControlValue(float pressure)
  {
    if (_tablet_control == eTabletControl_On)
    {
      if (_tablet_active_group == eTerrainTabletActiveGroup_Radius)
      {
        _radius = std::max(0.0f, std::min(1000.0f, pressure / 20.48f));
        _radius_slider->value = _radius / 1000.0f;
      }
      else if (_tablet_active_group == eTerrainTabletActiveGroup_Speed)
      {
        _speed = std::max(0.0f, std::min(10.0f, pressure / 204.8f));
        _speed_slider->value = _speed / 10.0f;
      }
    }
  }
}

