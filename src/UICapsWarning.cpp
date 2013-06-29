#include "UICapsWarning.h"

#include <algorithm>

#include "Noggit.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UIButton.h"
#include "UITexture.h"
#include "MapView.h"
#include "Video.h" // video
 
#include "Log.h"



UICapsWarning::UICapsWarning( MapView *mapview )
  : UIWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f - ( video.yres() /4), winWidth, winHeight )
{
  addChild( new UITexture( 10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Sigil_Thorim.blp" ) );
  addChild( new UIText( 95.0f, 20.0f, "Caps lock in on!", app.getArial14(), eJustifyLeft ) );
  addChild( new UIText( 95.0f, 40.0f, "Turn it off to work with noggit.", app.getArial14(), eJustifyLeft ) );
}

void UICapsWarning::resize()
{
  x( std::max( ( video.xres() / 2.0f ) - ( winWidth / 2.0f ), 0.0f ) );
  y( std::max( ( video.yres() / 2.0f ) - ( winHeight / 2.0f ), 0.0f )  - ( video.yres() /4) );
}

