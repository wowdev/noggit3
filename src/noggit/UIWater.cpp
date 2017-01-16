// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/UIWater.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <noggit/Environment.h>
#include <noggit/application.h> // fonts
#include <noggit/UIText.h>
#include <noggit/UIButton.h>
#include <noggit/UICheckBox.h>
#include <noggit/UITexture.h>
#include <noggit/UISlider.h>
#include <noggit/UIMapViewGUI.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/Video.h> // video
#include <iostream>
#include <sstream>
#include <noggit/UIWaterTypeBrowser.h>

#include <noggit/Log.h>

void toggleDisplayAllLayers(bool b, int);

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
		boost::bind(&UIWater::changeWaterHeight, this, _1, _2),
		-100)
		);

	addChild(new UIButton(45.0f, 90.0f, 23.0f, 30.0f,
		"-10",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::changeWaterHeight, this, _1, _2),
		-10)
		);

	addChild(new UIButton(70.0f, 90.0f, 19.0f, 30.0f,
		"-1",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::changeWaterHeight, this, _1, _2),
		-1)
		);

	addChild(new UIButton(91.0f, 90.0f, 19.0f, 30.0f,
		"+1",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::changeWaterHeight, this, _1, _2),
		1)
		);

	addChild(new UIButton(112.0f, 90.0f, 23.0f, 30.0f,
		"+10",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::changeWaterHeight, this, _1, _2),
		10)
		);

	addChild(new UIButton(137.0f, 90.0f, 38.0f, 30.0f,
		"+100",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::changeWaterHeight, this, _1, _2),
		100)
		);
	addChild(new UIText(5.0f, 108.f, "numpad +/- (alt *2, ctrl *5)", app.getArial12(), eJustifyLeft));

	waterType = new UIButton(5.0f, 135.0f, 170.0f, 30.0f,
		"Type: none",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::openWaterTypeBrowser, this, _1, _2),
		100);

	addChild(waterType);

	waterGenFactor = new UISlider(5.0f, 185.0f, 169.0f, 100.0f, 0.0f);
	waterGenFactor->setValue(0.5f);
	waterGenFactor->setText("Opacity Gen Factor: ");
	addChild(waterGenFactor);

	waterGen = new UIButton(5.0f, 205.0f, 170.0f, 30.0f,
		"Auto Opacity",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::autoGen, this, _1, _2),
		100);

	addChild(waterGen);

	addWater = new UIButton(5.0f, 225.0f, 170.0f, 30.0f,
		"Fill with water",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::AddWater, this, _1, _2),
		100);
	addChild(addWater);

	cropWater = new UIButton(5.0f, 245.0f, 170.0f, 30.0f,
		"Crop water",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
		"Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
		boost::bind(&UIWater::CropWater, this, _1, _2),
		100);
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
    [this] (UIFrame::Ptr, int)
    {
      size_t layer = std::max(0, Environment::getInstance()->currentWaterLayer - 1);
      waterLayer->setText(std::to_string(layer + 1));
      Environment::getInstance()->currentWaterLayer = layer;
    },
    0)
    );

  addChild(new UIButton(102.0f, 320.0f, 50.0f, 30.0f,
    ">>",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    [this](UIFrame::Ptr, int)
    {
      size_t layer = std::min(4, Environment::getInstance()->currentWaterLayer + 1);
      waterLayer->setText(std::to_string(layer + 1));
      Environment::getInstance()->currentWaterLayer = layer;
    },
    0)
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

void UIWater::resize()
{
	// fixed so no resize
}

void UIWater::setWaterTrans(float val)
{
	if (std::fmod(val, 0.1f) > 0.1f) return; //reduce performence hit
	gWorld->setWaterTrans(tile, (unsigned char)val);
}

void UIWater::addWaterLayer(UIFrame::Ptr /*ptr*/, int /*someint*/)
{
	gWorld->addWaterLayer(tile, 0.0f, (unsigned char)(waterOpacity->value * 255));
}

void UIWater::deleteWaterLayer(UIFrame::Ptr /*ptr*/, int /*someint*/)
{
	gWorld->deleteWaterLayer(tile);
}

void UIWater::setWaterHeight(float val)
{
	if (std::fmod(val, 0.1f) > 0.1f) return; //reduce performence hit
	gWorld->setWaterHeight(tile, val);
}

void UIWater::changeWaterHeight(UIFrame::Ptr /*ptr*/, int someint)
{
	gWorld->setWaterHeight(tile, ((float)someint));
	updateData();
}

void UIWater::openWaterTypeBrowser(UIFrame::Ptr /*ptr*/, int someint)
{
	if (this->mainGui->guiWaterTypeSelector->hidden())
		this->mainGui->guiWaterTypeSelector->show();
	else
		this->mainGui->guiWaterTypeSelector->hide();
}

void UIWater::changeWaterType(int waterint)
{
	gWorld->setWaterType(tile, waterint);
	updateData();
}

void UIWater::autoGen(UIFrame::Ptr ptr, int someint)
{
	gWorld->autoGenWaterTrans(tile, (int)waterGenFactor->value * 100);
	updateData();
}

void UIWater::AddWater(UIFrame::Ptr ptr, int someint)
{
	gWorld->AddWaters(tile);
	updateData();
}

void UIWater::CropWater(UIFrame::Ptr ptr, int someint)
{
	gWorld->CropWaterADT(tile);
	updateData();
}

void toggleDisplayAllLayers(bool b, int)
{
  Environment::getInstance()->displayAllWaterLayers = b;
}
