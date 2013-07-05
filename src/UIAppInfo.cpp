#include "UIAppInfo.h"

#include <string>

#include "Model.h" // Model
#include "ModelManager.h" // ModelManager
#include "Noggit.h" // app.getArial14()
#include "UIMapViewGUI.h" // UIMapViewGUI
#include "UIModel.h" // UIModel
#include "UIText.h" // UIText

UIAppInfo::UIAppInfo( float xPos, float yPos, float w, float h, UIMapViewGUI* setGui )
: UICloseWindow( xPos, yPos, w, h, "Application Info", true )
, mainGui( setGui )
, theInfos( new UIText( 8.0f, 20.0f, app.getArial14(), eJustifyLeft ) )
, mModelToLoad( "World\\AZEROTH\\ELWYNN\\PASSIVEDOODADS\\Trees\\CanopylessTree01.m2" )
{
  this->addChild( this->theInfos );

 // UIModel* myTestmodel = new UIModel( 10.0f, 0.0f, w, h );
  //myTestmodel->setModel( ModelManager::add( mModelToLoad ) );
 // this->addChild( myTestmodel );
}

UIAppInfo::~UIAppInfo()
{
  ModelManager::delbyname( mModelToLoad );
}

void UIAppInfo::setText( const std::string& t )
{
  this->theInfos->setText( t );
}
