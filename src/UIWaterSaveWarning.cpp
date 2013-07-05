#include "UIWaterSaveWarning.h"

#include <algorithm>

#include "Noggit.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UIButton.h"
#include "UITexture.h"
#include "MapView.h"
#include "Video.h" // video
 
#include "Log.h"



UIWaterSaveWarning::UIWaterSaveWarning( MapView *mapview )
  : UIWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f - ( video.yres() / 2.5f), winWidth, winHeight )
{
  addChild( new UITexture( 10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\Ability_Creature_Poison_06.blp" ) );
  addChild( new UIText( 95.0f, 20.0f, "Old style water! Noggit will not", app.getArial14(), eJustifyLeft ) );
  addChild( new UIText( 95.0f, 40.0f, "save some water on this ADT!", app.getArial14(), eJustifyLeft ) );
}

void UIWaterSaveWarning::resize()
{
  x( std::max( ( video.xres() / 2.0f ) - ( winWidth / 2.0f ), 0.0f ) );
  y( std::max( ( video.yres() / 2.0f ) - ( winHeight / 2.0f ), 0.0f )  - ( video.yres() /4) );
}

