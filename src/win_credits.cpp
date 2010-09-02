#include "noggit.h"

#include "win_credits.h"

#include "MinimizeButton.h"
#include "textureUI.h"
#include "textUI.h"


winCredits::winCredits(  ) : closeWindowUI( ((float)video.xres/2) - (winWidth/2),((float)video.yres/2) - (winHeight/2),winWidth,winHeight,"")
{
	this->mustresize=true;
	// Icon
	textureUI* icon = new textureUI( 20.0f, 20.0f, 64.0f, 64.0f, video.textures.add("Interface\\ICONS\\INV_Potion_83.blp") );
	addChild( icon );
	// Titel and subtitel
	textUI*	text1 = new textUI( 73.0f, 24.0f, APP_TITLE, &skurri32, eJustifyLeft );
	addChild( text1 );
	textUI*	text2 = new textUI( 165.0f, 55.0f, APP_SUBTITLE, &fritz16, eJustifyLeft );
	addChild( text2 );

	textUI*	text3 = new textUI( 20.0f, 100.0f, "Ufoz [...], Cryect, Beket,Schlumpf, Tigurius, Steff", &fritz16, eJustifyLeft );
	addChild( text3 );
	textUI*	text4 = new textUI( 20.0f, 130.0f, "World of Warcraft is (C) Blizzard Entertainment", &fritz16, eJustifyLeft );
	addChild( text4 );

	textUI*	text5 = new textUI( 20.0f, 160.0f, APP_VERSION , &fritz16, eJustifyLeft );
	addChild( text5 );

	textUI*	text6 = new textUI( 360.0f, 160.0f, APP_DATE, &fritz16, eJustifyRight );
	addChild( text6 );
}

void winCredits::resize( )
{
	int newX = ((float)video.xres/2) - (winWidth/2);
	int newY = ((float)video.yres/2) - (winHeight/2);
	if(newX<0)newX=0;
	if(newY<0)newY=0;
	this->x = newX;
	this->y = newY;
	
}