#include "appInfo.h"

#include <string>

#include "Gui.h" // Gui
#include "MinimizeButton.h" // MinimizeButton
#include "model.h" // Model
#include "ModelManager.h" // ModelManager
#include "modelUI.h" // modelUI
#include "noggit.h" // arial14
#include "textUI.h" // textUI

appInfo::appInfo( float xPos, float yPos, float w, float h, Gui* setGui )
: closeWindowUI( xPos, yPos, w, h, "Application Info", true )
, mainGui( setGui )
, theInfos( new textUI( 8.0f, 7.0f, arial14, eJustifyLeft ) )
, mModelToLoad( "World\\AZEROTH\\ELWYNN\\PASSIVEDOODADS\\Trees\\CanopylessTree01.m2" )
{
  this->addChild( this->theInfos );

  modelUI* myTestmodel = new modelUI( 10.0f, 0.0f, w, h );
  myTestmodel->setModel( static_cast<Model*>( ModelManager::items[ModelManager::add( mModelToLoad )] ) );
  this->addChild( myTestmodel );
}

appInfo::~appInfo()
{
  ModelManager::delbyname( mModelToLoad );
}

void appInfo::setText( const std::string& t )
{
  this->theInfos->setText( t );
}
