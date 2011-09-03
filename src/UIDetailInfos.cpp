#include "UIDetailInfos.h"

#include <string>

#include "Noggit.h" // arial14
#include "UIMapViewGUI.h"
#include "UIMinimizeButton.h"
#include "UIText.h"

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
