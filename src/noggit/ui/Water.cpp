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
  , _liquid_id(2)
  , _radius(10.0f)
{
  addChild(new UIText(78.5f, 2.0f, "Water edit", app.getArial14(), eJustifyCenter));

  _radius_slider = new UISlider(5.0f, 35.0f, 170.0f, 250.0f, 0.0f);
  _radius_slider->setFunc([&](float f) { _radius = f;});
  _radius_slider->setValue(_radius / 250.0f);
  _radius_slider->setText("Radius:");
  addChild(_radius_slider);

  waterType = new UIButton(5.0f, 130.0f, 170.0f, 30.0f,
    "Type: none",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { mainGui->guiWaterTypeSelector->toggleVisibility(); }
    );

  addChild(waterType);

  addChild(new UIText(5.0f, 170.0f, "Auto transparency:", app.getArial12(), eJustifyLeft));
  addChild(new UIButton(5.0f, 190.0f, 75.0f, 30.0f,
    "River",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [] { gWorld->autoGenWaterTrans(0.0337f); } // value found by experimenting
  ));
  addChild(new UIButton(95.0f, 190.0f, 75.0f, 30.0f,
    "Ocean",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [] { gWorld->autoGenWaterTrans(0.007f); }
  ));

  cropWater = new UIButton(5.0f, 245.0f, 170.0f, 30.0f,
    "Crop water",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this]
    {
      gWorld->CropWaterADT(gWorld->camera);
      updateData();
    }
    );
  addChild(cropWater);

  addChild(new UICheckBox(5.0f, 270.0f, "Show all layers", &Environment::getInstance()->displayAllWaterLayers));

  UIText *txt = new UIText(5.0f, 300.0f, app.getArial12(), eJustifyLeft);
  txt->setText("Current layer:");
  addChild(txt);


  waterLayer = new UIText(90.0f, 322.0f, app.getArial12(), eJustifyCenter);
  waterLayer->setText(std::to_string(Environment::getInstance()->currentWaterLayer + 1));
  addChild(waterLayer);

  addChild(new UIButton(28.0f, 320.0f, 50.0f, 30.0f,
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

  addChild(new UIButton(102.0f, 320.0f, 50.0f, 30.0f,
    ">>",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this]
    {
      size_t layer = std::min(4, Environment::getInstance()->currentWaterLayer + 1);
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

void UIWater::paintLiquid(math::vector_3d const& pos, bool add)
{
  gWorld->paintLiquid(pos, _radius, _liquid_id, add);
}

void UIWater::changeRadius(float change)
{
  _radius = std::max(0.0f, std::min(250.0f, _radius + change));
  _radius_slider->setValue(_radius / 250.0f);
}