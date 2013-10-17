#include "UIWater.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "Noggit.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UIButton.h"
#include "UITexture.h"
#include "UISlider.h"
#include "MapView.h"
#include "Misc.h"
#include "World.h"
#include "Video.h" // video
#include <iostream>
#include <sstream>

#include "Log.h"



UIWater::UIWater( MapView *mapview ) 
  : UIWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f - ( video.yres() /4), winWidth, winHeight )
{
  addChild( new UIText( 78.5f, 2.0f, "Water edit", app.getArial14(), eJustifyCenter ) );


  waterOpercity = new UISlider(5.0f, 40.0f, 169.0f,255.0f,0.0f);
  waterOpercity->setValue(1);
  waterOpercity->setText("Opercity: ");
  waterOpercity->setFunc(boost::bind(&UIWater::setWaterTrans, this, _1));
  addChild(waterOpercity);
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

  waterType = new UIButton(5.0f, 130.0f, 170.0f, 30.0f,
    "Type: none",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
    "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
    boost::bind(&UIWater::changeWaterType, this, _1, _2),
    100);

  addChild(waterType);


  // Add dropdown type

  updateData();
}

void UIWater::updatePos(int newTileX, int newTileY)
{
  if(tileX == newTileX && tileY == newTileY) return;

  tileX = newTileX;
  tileY = newTileY;

  updateData();
}

void UIWater::updateData()
{
  std::stringstream ms;
  ms << gWorld->getWaterHeight(tileX, tileY) ;

  waterLevel->setText(ms.str());
  waterOpercity->value = (gWorld->getWaterTrans(tileX, tileY)/255.0f);

  std::stringstream mt;
  mt << gWorld->getWaterType(tileX, tileY) << " - " << LiquidTypeDB::getLiquidName(gWorld->getWaterType(tileX, tileY));

  waterType->setText(mt.str());
}

void UIWater::resize()
{
  // fixed so no resize
}

void UIWater::setWaterTrans(float val)
{
  if(std::fmod(val, 0.1f) > 0.1f) return; //reduce performence hit
  gWorld->setWaterTrans(tileX, tileY, val);
}

void UIWater::addWaterLayer(UIFrame::Ptr /*ptr*/, int /*someint*/)
{
  gWorld->addWaterLayer(tileX, tileY, 0.0f, waterOpercity->value * 255);
}

void UIWater::deleteWaterLayer(UIFrame::Ptr /*ptr*/, int /*someint*/)
{
  gWorld->deleteWaterLayer(tileX,tileY);
}

void UIWater::setWaterHeight(float val)
{
  if(std::fmod(val, 0.1f) > 0.1f) return; //reduce performence hit
  gWorld->setWaterHeight(tileX, tileY, val);
}

void UIWater::changeWaterHeight(UIFrame::Ptr /*ptr*/, int someint)
{
  gWorld->setWaterHeight(tileX, tileY, ( gWorld->getWaterHeight(tileX, tileY) + someint ) );
  updateData();
}

void UIWater::changeWaterType(UIFrame::Ptr /*ptr*/, int someint)
{
    // open type selector
}
