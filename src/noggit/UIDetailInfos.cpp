#include <noggit/UIDetailInfos.h>

#include <string>

#include <noggit/application.h> // arial14
#include <noggit/UIMapViewGUI.h>
#include <noggit/UIMinimizeButton.h>
#include <noggit/UIText.h>

UIDetailInfos::UIDetailInfos( float xPos, float yPos, float w, float h, UIMapViewGUI *setGui )
: UIWindow( xPos, yPos, w, h )
, mainGui( setGui )
, theInfos( new UIText( 8.0f, 7.0f, "", arial14, eJustifyLeft ) )
{
  addChild( new UIMinimizeButton( width() ) );
  addChild( theInfos );
}

void UIDetailInfos::setText( const std::string& t )
{
  theInfos->setText( t );
}
