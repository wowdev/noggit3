// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/FlattenTool.hpp>

#include <noggit/application.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/Slider.h>
#include <noggit/ui/TextBox.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/ToggleGroup.h>
#include <noggit/Misc.h>
#include <noggit/Environment.h>
#include <noggit/World.h>



namespace ui
{
  FlattenTool::FlattenTool(float x, float y)
    : UIWindow(x, y, (float)winWidth, (float)winHeight)
    , _radius(10.0f)
    , _speed(2.0f)
    , _angle(Environment::getInstance()->flattenAngle)
    , _orientation(Environment::getInstance()->flattenOrientation)
    , _locked(false)
    , _angled_mode(Environment::getInstance()->flattenAngleEnabled)
    , _flatten_type(eFlattenType_Linear)
    , _flatten_mode(eFlattenMode_Both)
  {
    addChild(new UIText(78.5f, 2.0f, "Flatten / Blur", app.getArial14(), eJustifyCenter));


    _type_toggle = new UIToggleGroup(&_flatten_type);
    addChild(new UICheckBox(6.0f, 15.0f, "Flat", _type_toggle, eFlattenType_Flat));
    addChild(new UICheckBox(80.0f, 15.0f, "Linear", _type_toggle, eFlattenType_Linear));
    addChild(new UICheckBox(6.0f, 40.0f, "Smooth", _type_toggle, eFlattenType_Smooth));
    _type_toggle->Activate(eFlattenType_Linear);

    _radius_slider = new UISlider(6.0f, 85.0f, 167.0f, 1000.0f, 0.00001f);
    _radius_slider->setFunc([&](float f) {_radius = f;});
    _radius_slider->setValue(_radius / 1000);
    _radius_slider->setText("Brush radius: ");
    addChild(_radius_slider);

    _speed_slider = new UISlider(6.0f, 110.0f, 167.0f, 10.0f, 0.00001f);
    _speed_slider->setFunc([&](float f) {_speed = f;});
    _speed_slider->setValue(_speed / 10.0f);
    _speed_slider->setText("Brush Speed: ");
    addChild(_speed_slider);


    addChild(new UIText(5.0f, 130.0f, "Flatten options:", app.getArial14(), eJustifyLeft));

    _angle_checkbox = new UICheckBox(6.0f, 150.0f, "Flatten Angle", [&](bool b, int) { _angled_mode = b; }, 0);
    _angle_checkbox->setState(_angled_mode);
    addChild(_angle_checkbox);

    _angle_slider = new UISlider(6.0f, 190.0f, 167.0f, 89.0f, 0.00001f);
    _angle_slider->setFunc( [&] (float f) { _angle = f; });
    _angle_slider->setValue(_angle / 89.0f);
    _angle_slider->setText("Angle: ");
    addChild(_angle_slider);

    _orientation_slider = new UISlider(6.0f, 220.0f, 167.0f, 360.0f, 0.00001f);
    _orientation_slider->setFunc( [&] (float f) { _orientation = f; });
    _orientation_slider->setValue(_orientation / 360.0f);
    _orientation_slider->setText("Orientation: ");
    addChild(_orientation_slider);


    _lock_checkbox = new UICheckBox(5.0f, 235.0f, "flatten relative to:", [&] (bool b, int) { _locked = b; }, 0);
    _lock_checkbox->setState(_locked);
    addChild(_lock_checkbox);

    addChild(new UIText(5.0f, 265.0f, "X:", app.getArial12(), eJustifyLeft));
    _lock_x = new UITextBox(50.0f, 265.0f, 100.0f, 30.0f, app.getArial12(), [&] (UITextBox::Ptr ptr, const std::string str) 
    {
      updateLockCoord(ptr, str, _lock_pos.x);
    });
    _lock_x->value(misc::floatToStr(_lock_pos.x));
    addChild(_lock_x);

    addChild(new UIText(5.0f, 285.0f, "Z:", app.getArial12(), eJustifyLeft));
    _lock_z = new UITextBox(50.0f, 285.0f, 100.0f, 30.0f, app.getArial12(), [&](UITextBox::Ptr ptr, const std::string str)
    {
      updateLockCoord(ptr, str, _lock_pos.z);
    });
    _lock_z->value(misc::floatToStr(_lock_pos.z));
    addChild(_lock_z);

    addChild(new UIText(5.0f, 305.0f, "Height:", app.getArial12(), eJustifyLeft));
    _lock_h = new UITextBox(50.0f, 305.0f, 100.0f, 30.0f, app.getArial12(), [&](UITextBox::Ptr ptr, const std::string str)
    {
      updateLockCoord(ptr, str, _lock_pos.y);
    });
    _lock_h->value(misc::floatToStr(_lock_pos.y));
    addChild(_lock_h);


    addChild(new UIText(5.0f, 330.0, "Flatten Type:", app.getArial14(), eJustifyLeft));

    _mode_toggle = new UIToggleGroup(&_flatten_mode);
    addChild(new UICheckBox(5.0f, 345.0f, "Raise/Lower", _mode_toggle, eFlattenMode_Both));
    addChild(new UICheckBox(105.0f, 345.0f, "Raise", _mode_toggle, eFlattenMode_Raise));
    addChild(new UICheckBox(5.0f, 370.0f, "Lower", _mode_toggle, eFlattenMode_Lower));
    _mode_toggle->Activate(eFlattenMode_Both);
  }

  void FlattenTool::flatten(float dt)
  {
    math::vector_3d const& pos = Environment::getInstance()->get_cursor_pos();

    gWorld->flattenTerrain ( pos.x
                           , pos.z
                           , pow (0.5f, dt *_speed)
                           , _radius
                           , _flatten_type
                           , _flatten_mode
                           , _locked ? _lock_pos : pos
                           , _angled_mode ? _angle : 0.0f
                           , _angled_mode ? _orientation : 0.0f
                           );
  }

  void FlattenTool::blur(float dt)
  {
    math::vector_3d const& pos = Environment::getInstance()->get_cursor_pos();

    gWorld->blurTerrain(pos.x, pos.z, pow(0.5, dt*_speed), _radius, _flatten_type);
  }

  void FlattenTool::nextFlattenType()
  {
    _flatten_type = (++_flatten_type) % eFlattenType_Count;
    _type_toggle->Activate(_flatten_type);
  }

  void FlattenTool::nextFlattenMode()
  {
    _flatten_mode = (++_flatten_mode) % eFlattenMode_Count;
    _mode_toggle->Activate(_flatten_mode);
  }

  void FlattenTool::toggleFlattenAngle()
  {
    _angled_mode = !_angled_mode;
    _angle_checkbox->setState(_angled_mode);
  }

  void FlattenTool::toggleFlattenLock()
  {
    _locked = !_locked;
    _lock_checkbox->setState(_locked);
  }
  
  void FlattenTool::lockPos()
  {
    _lock_pos = Environment::getInstance()->get_cursor_pos();
    _lock_x->value(misc::floatToStr(_lock_pos.x));
    _lock_z->value(misc::floatToStr(_lock_pos.z));
    _lock_h->value(misc::floatToStr(_lock_pos.y));
  }

  void FlattenTool::changeRadius(float change)
  {
    _radius = std::max(0.0f, std::min(1000.0f, _radius + change));
    _radius_slider->setValue(_radius / 1000.0f);
  }

  void FlattenTool::changeSpeed(float change)
  {
    _speed = std::max(0.0f, std::min(10.0f, _speed + change));
    _speed_slider->setValue(_speed / 10.0f);
  }

  void FlattenTool::changeOrientation(float change)
  {
    _orientation += change;
    
    if (_orientation < 0.0f)
    {
      _orientation += 360.0f;
    }
    else if (_orientation > 360.0f)
    {
      _orientation -= 360.0f;
    }

    _orientation_slider->setValue(_orientation / 360.0f);
  }

  void FlattenTool::changeAngle(float change)
  {
    _angle = std::max(0.0f, std::min(89.0f, _angle + change));
    _angle_slider->setValue(_angle / 90.0f);
  }

  void FlattenTool::changeHeight(float change)
  {
    _lock_pos.y += change;
    _lock_h->value(misc::floatToStr(_lock_pos.y));
  }

  void FlattenTool::setRadius(float radius)
  {
    _radius = std::max(0.0f, std::min(1000.0f, radius));
    _radius_slider->setValue(_radius / 1000.0f);
  }

  void FlattenTool::updateLockCoord(UITextBox* cb, std::string const& str, float& value)
  {
    try
    {
      float val = std::atof(str.c_str());
      value = val;
      cb->value(misc::floatToStr(val));
    }
    catch (std::exception const & e)
    {
      cb->value(misc::floatToStr(value));
    }
  }
}

