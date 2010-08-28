#include "appInfo.h"
#include "video.h"
#include "MinimizeButton.h"
#include "noggit.h"
#include "modelUI.h"
#include "model.h"

appInfo::appInfo(float xPos, float yPos, float w, float h, Gui *setGui) : window(xPos, yPos, w, h)
{
	this->mainGui = setGui;

	this->addChild( reinterpret_cast<frame*>( new MinimizeButton( w, this ) ) );

	this->theInfos = new textUI( 8.0f, 7.0f, &arial14, eJustifyLeft );
	this->addChild( this->theInfos );

	modelUI* myTestmodel = new modelUI( 10.0f, 0.0f, w, h );
	myTestmodel->setModel( new Model( "World\\AZEROTH\\ELWYNN\\PASSIVEDOODADS\\Trees\\CanopylessTree01.m2" ) );
	this->addChild( myTestmodel );
}

void appInfo::setText( std::string t )
{
	this->theInfos->setText( t );
}