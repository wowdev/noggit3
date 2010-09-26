#include "noggit.h" // fonts

#include "win_credits.h"

#include "MinimizeButton.h"
#include "textureUI.h"
#include "textUI.h"

#include "video.h" // video

winCredits::winCredits( ) : closeWindowUI( ((float)video.xres/2) - (winWidth/2),((float)video.yres/2) - (winHeight/2),winWidth,winHeight,"")
{
	this->mustresize = true;
	addChild( new textureUI( 20.0f, 20.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Potion_83.blp" ) );
	addChild( new textUI( 73.0f, 24.0f, APP_TITLE, &skurri32, eJustifyLeft ) );
	addChild( new textUI( 165.0f, 55.0f, APP_SUBTITLE, &fritz16, eJustifyLeft ) );
	addChild( new textUI( 20.0f, 100.0f, "Ufoz [...], Cryect, Beket, Schlumpf, Tigurius, Steff", &fritz16, eJustifyLeft ) );
	addChild( new textUI( 20.0f, 130.0f, "World of Warcraft is (C) Blizzard Entertainment", &fritz16, eJustifyLeft ) );
	addChild( new textUI( 20.0f, 160.0f, APP_VERSION , &fritz16, eJustifyLeft ) );
	addChild( new textUI( 360.0f, 160.0f, APP_DATE, &fritz16, eJustifyRight ) );
}

void winCredits::resize()
{
  using std::max;
	this->x = max( ( (float)video.xres / 2.0f ) - ( winWidth / 2.0f ), 0.0f );
	this->y = max( ( (float)video.yres / 2.0f ) - ( winHeight / 2.0f ), 0.0f );
	
}
