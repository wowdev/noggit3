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
{
  addChild(new UIText(78.5f, 2.0f, "Water edit", app.getArial14(), eJustifyCenter));


  waterOpacity = new UISlider(5.0f, 40.0f, 169.0f, 255.0f, 0.0f);
  waterOpacity->setValue(1);
  waterOpacity->setText("Opacity: ");
  waterOpacity->setFunc(boost::bind(&UIWater::setWaterTrans, this, _1));
  addChild(waterOpacity);
  addChild(new UIText(5.0f, 50.0f, "shift + numpad +/-", app.getArial12(), eJustifyLeft));

  addChild(new UIText(60.0f, 74.0f, "Level: ", app.getArial12(), eJustifyLeft));
  waterLevel = new UIText(95.0f, 74.0f, "0.0", app.getArial12(), eJustifyLeft);
  addChild(waterLevel);

  addChild(new UIButton(5.0f, 90.0f, 38.0f, 30.0f,
    "-100",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { changeWaterHeight (-100.f); }
    ));

  addChild(new UIButton(45.0f, 90.0f, 23.0f, 30.0f,
    "-10",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { changeWaterHeight (-10.f); }
    ));

  addChild(new UIButton(70.0f, 90.0f, 19.0f, 30.0f,
    "-1",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { changeWaterHeight (-1.f); }
    ));

  addChild(new UIButton(91.0f, 90.0f, 19.0f, 30.0f,
    "+1",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { changeWaterHeight (1.f); }
    ));

  addChild(new UIButton(112.0f, 90.0f, 23.0f, 30.0f,
    "+10",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { changeWaterHeight (10.f); }
    ));

  addChild(new UIButton(137.0f, 90.0f, 38.0f, 30.0f,
    "+100",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { changeWaterHeight (100.f); }
    ));
  addChild(new UIText(5.0f, 108.f, "numpad +/- (alt *2, ctrl *5)", app.getArial12(), eJustifyLeft));

  waterType = new UIButton(5.0f, 135.0f, 170.0f, 30.0f,
    "Type: none",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this] { mainGui->guiWaterTypeSelector->toggleVisibility(); }
    );

  addChild(waterType);

  waterGenFactor = new UISlider(5.0f, 185.0f, 169.0f, 100.0f, 0.0f);
  waterGenFactor->setValue(0.5f);
  waterGenFactor->setText("Opacity Gen Factor: ");
  addChild(waterGenFactor);

  waterGen = new UIButton(5.0f, 205.0f, 170.0f, 30.0f,
    "Auto Opacity",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this]
    {
      gWorld->autoGenWaterTrans(tile, (int)waterGenFactor->value * 100);
      updateData();
    }
    );

  addChild(waterGen);

  addWater = new UIButton(5.0f, 225.0f, 170.0f, 30.0f,
    "Fill with water",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this]
    {
      gWorld->AddWaters(tile);
      updateData();
    }
    );
  addChild(addWater);

  cropWater = new UIButton(5.0f, 245.0f, 170.0f, 30.0f,
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

  displayAllLayers = new UICheckBox(5.0f, 270.0f, "Show all layers", [] (bool b, int)
  {
    Environment::getInstance()->displayAllWaterLayers = b;
  }, 0);
  displayAllLayers->setState(Environment::getInstance()->displayAllWaterLayers);
  addChild(displayAllLayers);

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

void UIWater::updatePos(int tileX, int tileZ)
{
  if (tile.x == tileX && tile.z == tileZ) return;

  tile = tile_index(tileX, tileZ);

  updateData();
}

void UIWater::updateData()
{
  float h = gWorld->HaveSelectWater(tile);
  if (h)
  {
    std::stringstream ms;
    ms << h;
    waterLevel->setText(ms.str());
  }
  else
  {
    std::stringstream ms;
    ms << gWorld->getWaterHeight(tile);
    waterLevel->setText(ms.str());
  }
  waterOpacity->value = (gWorld->getWaterTrans(tile) / 255.0f);

  std::stringstream mt;
  mt << gWorld->getWaterType(tile) << " - " << LiquidTypeDB::getLiquidName(gWorld->getWaterType(tile));

  waterType->setText(mt.str());
}

void UIWater::setWaterTrans(float val)
{
  if (std::fmod(val, 0.1f) > 0.1f) return; //reduce performence hit
  gWorld->setWaterTrans(tile, (unsigned char)val);
}

void UIWater::changeWaterHeight (float height)
{
  gWorld->setWaterHeight(tile, height);
  updateData();
}

void UIWater::changeWaterType(int waterint)
{
  gWorld->setWaterType(tile, waterint);
  updateData();
}
