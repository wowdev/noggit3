#include "win_credits.h"

#include <algorithm>

#include "noggit.h" // fonts
#include "revision.h"

#include "MinimizeButton.h"
#include "textureUI.h"
#include "textUI.h"

#include "video.h" // video

winCredits::winCredits( ) : closeWindowUI( video.xres / 2.0f - winWidth / 2.0f, video.yres / 2.0f - winHeight / 2.0f, winWidth, winHeight, "" )
{
	addChild( new textureUI( 20.0f, 20.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Potion_83.blp" ) );
	addChild( new textUI( 73.0f, 24.0f, "Noggit Studio", skurri32, eJustifyLeft ) );
	addChild( new textUI( 165.0f, 55.0f, "a wow map editor", fritz16, eJustifyLeft ) );
	addChild( new textUI( 20.0f, 100.0f, "Ufoz [...], Cryect, Beket, Schlumpf, Tigurius, Steff", fritz16, eJustifyLeft ) );
	addChild( new textUI( 20.0f, 130.0f, "World of Warcraft is (C) Blizzard Entertainment", fritz16, eJustifyLeft ) );
	addChild( new textUI( 20.0f, 160.0f, STRPRODUCTVER , fritz16, eJustifyLeft ) );
	addChild( new textUI( 360.0f, 160.0f, __DATE__ ", " __TIME__, fritz16, eJustifyRight ) );
}

void winCredits::resize()
{
	using std::max;
	this->x = max( ( video.xres / 2.0f ) - ( winWidth / 2.0f ), 0.0f );
	this->y = max( ( video.yres / 2.0f ) - ( winHeight / 2.0f ), 0.0f );
}
