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

#include "Log.h"



UIWater::UIWater( MapView *mapview ) 
  : UIWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f - ( video.yres() /4), winWidth, winHeight )
{
  addChild( new UIText( 78.5f, 2.0f, "Water edit", app.getArial14(), eJustifyCenter ) );

  // addChild( new UITexture( 10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Misc_QuestionMark.blp" ) );

  // addChild( new UIText( 95.0f, 40.0f, "Changes will be lost!", app.getArial14(), eJustifyLeft ) );


  waterOpercity = new UISlider(8.0f, 40.0f, 167.0f,255.0f,0.0f);
  waterOpercity->setValue(1);
  waterOpercity->setText("Opercity: ");
  waterOpercity->setFunc(boost::bind(&UIWater::setWaterTrans, this, _1));
  addChild(waterOpercity);

  waterLevel = new UISlider(8.0f, 65.0f, 167.0f, 200.0f, -100.0f);
  waterLevel->setValue(0.0f);
  waterLevel->setText("Level: ");
  waterLevel->setFunc(boost::bind(&UIWater::setWaterHeight, this, _1));
  addChild(waterLevel);

  addChild(new UIButton(8.0f, 120.0f, 100.0f, 30.0f,
                        "Fill Tile",
                        "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
                        "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
                        boost::bind(&UIWater::addWaterLayer, this, _1, _2),
                        2)
           );

  addChild(new UIButton(8.0f, 160.0f, 100.0f, 30.0f,
                        "Clear Tile",
                        "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp",
                        "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp",
                        boost::bind(&UIWater::deleteWaterLayer, this, _1, _2),
                        2)
           );

  // Add dropdown type

  // Add button Delete

  // Add button New
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
  waterLevel->setValue((gWorld->getWaterHeight(tileX, tileY) + 100.0f)/200.0f);
  waterOpercity->setValue(gWorld->getWaterTrans(tileX, tileY)/255.0f);
}

void UIWater::resize()
{
  // fixed so no resize
}

void UIWater::setWaterTrans(float val)
{
  gWorld->setWaterTrans(tileX, tileY, val);
}

void UIWater::addWaterLayer(UIFrame::Ptr /*ptr*/, int /*someint*/)
{
  gWorld->addWaterLayer(tileX, tileY, waterLevel->value * 200.0f - 100.0f, waterOpercity->value * 255);
}

void UIWater::deleteWaterLayer(UIFrame::Ptr /*ptr*/, int /*someint*/)
{
  gWorld->deleteWaterLayer(tileX,tileY);
}

void UIWater::setWaterHeight(float val)
{
  gWorld->setWaterHeight(tileX, tileY, val);
}



