#include "detailInfos.h"
#include "MinimizeButton.h"
#include "noggit.h" // arial14
#include "Gui.h"
#include "textUI.h"

detailInfos::detailInfos( float xPos, float yPos, float w, float h, Gui *setGui ) : window( xPos, yPos, w, h )
{
	this->mainGui = setGui;

	this->addChild( reinterpret_cast<frame*>( new MinimizeButton( w, this ) ) );

	this->theInfos = new textUI( 8.0f, 7.0f, "", &arial14, eJustifyLeft );
	this->addChild( this->theInfos );
}

void detailInfos::setText( const std::string& t )
{
	this->theInfos->setText( t );
}
