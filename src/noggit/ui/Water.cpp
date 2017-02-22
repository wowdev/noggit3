// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Water.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <noggit/Environment.h>
#include <noggit/application.h> // fonts
#include <noggit/ui/Text.h>
#include <noggit/ui/Button.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Texture.h>
#include <noggit/ui/ToggleGroup.h>
#include <noggit/ui/Slider.h>
#include <noggit/ui/MapViewGUI.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/Video.h> // video
#include <iostream>
#include <sstream>
#include <noggit/ui/WaterTypeBrowser.h>

#include <noggit/Log.h>

UIWater::UIWater(UIMapViewGUI *setGui)
  : UIWindow((float)video.xres() / 2.0f - (float)winWidth / 2.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f - (float)(video.yres() / 4), (float)winWidth, (float)winHeight)
  , mainGui(setGui)
  , tile(0, 0)
  , _liquid_id(5)
  , _radius(10.0f)
  , _angle(10.0f)
  , _orientation(0.0f)
  , _locked(false)
  , _angled_mode(false)
  , _override_liquid_id(true)
  , _override_height(true)
  , _opacity_mode(river_opacity)
  , _custom_opacity_factor(0.0337f)
  , _lock_pos(math::vector_3d(0.0f, 0.0f, 0.0f))
{
  addChild(new UIText(78.5f, 2.0f, "Water edit", app.getArial14(), eJustifyCenter));

  _radius_slider = new UISlider(5.0f, 35.0f, 170.0f, 250.0f, 0.0f);
  _radius_slider->setFunc([&](float f) { _radius = f;});
  _radius_slider->setValue(_radius / 250.0f);
  _radius_slider->setText("Radius:");
  addChild(_radius_slider);

  _angle_slider = new UISlider(6.0f, 65.0f, 167.0f, 89.0f, 0.00001f);
  _angle_slider->setFunc([&](float f) { _angle = f; });
  _angle_slider->setValue(_angle / 89.0f);
  _angle_slider->setText("Angle: ");
  addChild(_angle_slider);

  _orientation_slider = new UISlider(6.0f, 95.0f, 167.0f, 360.0f, 0.00001f);
  _orientation_slider->setFunc([&](float f) { _orientation = f; });
  _orientation_slider->setValue(_orientation / 360.0f);
  _orientation_slider->setText("Orientation: ");
  addChild(_orientation_slider);

  addChild(_angle_checkbox = new UICheckBox(5.0f, 110.0f, "Angled water", &_angled_mode));
  addChild(_lock_checkbox = new UICheckBox(5.0f, 135.0f, "Lock position", &_locked));

  waterType = new UIButton(5.0f, 170.0f, 170.0f, 30.0f,
    "Type: none",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { mainGui->guiWaterTypeSelector->toggleVisibility(); }
    );

  addChild(waterType);

  addChild(new UIText(5.0f, 200.0f, "Override :", app.getArial12(), eJustifyLeft));

  addChild(new UICheckBox(5.0f, 215.0f, "Liquid ID", &_override_liquid_id));
  addChild(new UICheckBox(95.0f, 215.0f, "Height", &_override_height));

  addChild(new UIText(5.0f, 245.0f, "Auto opacity:", app.getArial12(), eJustifyLeft));

  UIToggleGroup *transparency_toggle = new UIToggleGroup(&_opacity_mode);

  addChild(new UICheckBox(5.0f, 260.0f, "River", transparency_toggle, river_opacity));
  addChild(new UICheckBox(95.0f, 260.0f, "Ocean", transparency_toggle, ocean_opacity));
  addChild(new UICheckBox(5.0f, 285.0f, "", transparency_toggle, custom_opacity));

  transparency_toggle->Activate(river_opacity);

  UISlider *opacity_slider = new UISlider(35.0f, 300.0f, 140.0f, 10.0f, 0.0f);
  opacity_slider->setValue(_custom_opacity_factor * 10.0f);
  opacity_slider->setText("custom factor:");
  opacity_slider->setFunc([&](float f) { _custom_opacity_factor = f * 0.01f; });
  addChild(opacity_slider);

  addChild(new UIButton(5.0f, 320.0f, 170.0f, 30.0f,
    "Regen ADT opacity",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
                       [this] { gWorld->autoGenWaterTrans(tile, get_opacity_factor()); }
  ));

  cropWater = new UIButton(5.0f, 350.0f, 170.0f, 30.0f,
    "Crop water",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this]
    {
      gWorld->CropWaterADT(tile);
      updateData();
    }
    );
  addChild(cropWater);

  addChild(new UICheckBox(5.0f, 370.0f, "Show all layers", &Environment::getInstance()->displayAllWaterLayers));

  UIText *txt = new UIText(5.0f, 400.0f, app.getArial12(), eJustifyLeft);
  txt->setText("Current layer:");
  addChild(txt);


  waterLayer = new UIText(90.0f, 422.0f, app.getArial12(), eJustifyCenter);
  waterLayer->setText(std::to_string(Environment::getInstance()->currentWaterLayer + 1));
  addChild(waterLayer);

  addChild(new UIButton(28.0f, 420.0f, 50.0f, 30.0f,
    "<<",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this]
    {
      size_t layer = std::max(0, Environment::getInstance()->currentWaterLayer - 1);
      waterLayer->setText(std::to_string(layer + 1));
      Environment::getInstance()->currentWaterLayer = layer;
    }
    ));

  addChild(new UIButton(102.0f, 420.0f, 50.0f, 30.0f,
    ">>",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this]
    {
      size_t layer = Environment::getInstance()->currentWaterLayer + 1;
      waterLayer->setText(std::to_string(layer + 1));
      Environment::getInstance()->currentWaterLayer = layer;
    })
  );

  updateData();
}

void UIWater::updatePos(tile_index const& newTile)
{
  if (newTile == tile) return;

  tile = newTile;

  updateData();
}

void UIWater::updateData()
{
  std::stringstream mt;
  mt << _liquid_id << " - " << LiquidTypeDB::getLiquidName(_liquid_id);
  waterType->setText(mt.str());
}

void UIWater::changeWaterType(int waterint)
{
  _liquid_id = waterint;
  updateData();
}

void UIWater::changeRadius(float change)
{
  _radius = std::max(0.0f, std::min(250.0f, _radius + change));
  _radius_slider->setValue(_radius / 250.0f);
}

void UIWater::changeOrientation(float change)
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

void UIWater::changeAngle(float change)
{
  _angle = std::max(0.0f, std::min(89.0f, _angle + change));
  _angle_slider->setValue(_angle / 90.0f);
}

void UIWater::paintLiquid(math::vector_3d const& pos, bool add)
{
  gWorld->paintLiquid( pos
                     , _radius
                     , _liquid_id
                     , add
                     , math::degrees(_angled_mode ? _angle : 0.0f)
                     , math::degrees(_angled_mode ? _orientation : 0.0f)
                     , _locked
                     , _lock_pos
                     , _override_height
                     , _override_liquid_id
                     , get_opacity_factor()
                     );
}

void UIWater::lockPos(math::vector_3d const& cursor_pos)
{ 
  _lock_pos = cursor_pos;

  if (!_locked)
  {
    toggle_lock();
  }
}

void UIWater::toggle_lock()
{
  _locked = !_locked;
  _lock_checkbox->setState(_locked);
}

void UIWater::toggle_angled_mode()
{
  _angled_mode = !_angled_mode;
  _angle_checkbox->setState(_angled_mode);
}

float UIWater::get_opacity_factor() const
{
  switch (_opacity_mode)
  {
  default:          // values found by experimenting
  case river_opacity:  return 0.0337f;
  case ocean_opacity:  return 0.007f;
  case custom_opacity: return _custom_opacity_factor;
  }
}