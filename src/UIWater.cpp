#include "UIWater.h"

#include <algorithm>

#include "Noggit.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UIButton.h"
#include "UITexture.h"
#include "UISlider.h"
#include "MapView.h"
#include "misc.h"
#include "World.h"
#include "Video.h" // video

#include "Log.h"



UIWater::UIWater( MapView *mapview ) 
  : UIWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f - ( video.yres() /4), winWidth, winHeight )
{
  addChild( new UIText( 78.5f, 2.0f, "Water edit", app.getArial14(), eJustifyCenter ) );

 // addChild( new UITexture( 10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Misc_QuestionMark.blp" ) );

 // addChild( new UIText( 95.0f, 40.0f, "Changes will be lost!", app.getArial14(), eJustifyLeft ) );
  //addChild( new UIButton( this->width() - 120.0f, 80.0f, 100.0f, 30.0f, "Cancel", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", closeme, 2 ) );

  waterOpercity = new UISlider(8.0f,40.0f,167.0f,99.0f,1.0f);
 // waterOpercity->setFunc(setWaterOpercity); // Ask Hanferfor binding.
  waterOpercity->setValue(1);
  //waterOpercity->setFunc()
  waterOpercity->setText( "Opercity: " );
  addChild(waterOpercity);

  waterLevel = new UISlider(8.0f,65.0f,167.0f,99.0f,1.0f);
  //waterOpercity->setFunc(setGroundBrushRadius);
  waterLevel->setValue(1);
  waterLevel->setText( "Level: " );
  addChild(waterLevel);

  // Add dropdown type

  // Add button Delete

  // Add button New
}

void UIWater::resize()
{
  // fixed so no resize
}

void UIWater::setWaterOpercity( float level )
{
  gWorld->setWaterOpercity(misc::FtoIround((gWorld->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((gWorld->camera.z-(TILESIZE/2))/TILESIZE),50.0f);
}




