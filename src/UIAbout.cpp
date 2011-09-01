#include "UIAbout.h"

#include <algorithm>

#include "Noggit.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UITexture.h"
#include "Video.h" // video

#include "Log.h"

UIAbout::UIAbout( )
: UICloseWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f, winWidth, winHeight, "" )
{
  addChild( new UITexture( 20.0f, 20.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Potion_83.blp" ) );
  addChild( new UIText( 73.0f, 24.0f, "Noggit Studio", skurri32, eJustifyLeft ) );
  addChild( new UIText( 165.0f, 55.0f, "a wow map editor", fritz16, eJustifyLeft ) );
  addChild( new UIText( 20.0f, 100.0f, "Ufoz [...],   Cryect,   Beket,   Schlumpf,   Tigurius", fritz16, eJustifyLeft ) );
  addChild( new UIText( 120.0f, 120.0f, " Steff,  Garthog,   .......", fritz16, eJustifyLeft ) );
  addChild( new UIText( 20.0f, 160.0f, "World of Warcraft is (C) Blizzard Entertainment", fritz16, eJustifyLeft ) );
  addChild( new UIText( 20.0f, 190.0f, STRPRODUCTVER , fritz16, eJustifyLeft ) );
  addChild( new UIText( 375.0f, 190.0f, __DATE__ ", " __TIME__, fritz16, eJustifyRight ) );
}

void UIAbout::resize()
{
  x( std::max( ( video.xres() / 2.0f ) - ( winWidth / 2.0f ), 0.0f ) );
  y( std::max( ( video.yres() / 2.0f ) - ( winHeight / 2.0f ), 0.0f ) );
}
