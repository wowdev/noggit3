#include <noggit/UIAppInfo.h>

#include <string>

#include <noggit/Model.h> // Model
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/application.h> // arial14
#include <noggit/UIMapViewGUI.h> // UIMapViewGUI
#include <noggit/UIModel.h> // UIModel
#include <noggit/UIText.h> // UIText

UIAppInfo::UIAppInfo( float xPos, float yPos, float w, float h, UIMapViewGUI* setGui )
: UICloseWindow( xPos, yPos, w, h, "Application Info", true )
, mainGui( setGui )
, theInfos( new UIText( 8.0f, 20.0f, arial14, eJustifyLeft ) )
, mModelToLoad( "World\\AZEROTH\\ELWYNN\\PASSIVEDOODADS\\Trees\\CanopylessTree01.m2" )
{
  addChild( theInfos );

 // UIModel* myTestmodel = new UIModel( 10.0f, 0.0f, w, h );
  //myTestmodel->setModel( ModelManager::add( mModelToLoad ) );
 // addChild( myTestmodel );
}

UIAppInfo::~UIAppInfo()
{
  ModelManager::delbyname( mModelToLoad );
}

void UIAppInfo::setText( const std::string& t )
{
  theInfos->setText( t );
}
