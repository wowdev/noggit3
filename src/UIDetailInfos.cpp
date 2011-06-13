#include "UIDetailInfos.h"

#include <string>

#include "Noggit.h" // arial14
#include "UIMapViewGUI.h"
#include "UIMinimizeButton.h"
#include "UIText.h"

UIDetailInfos::UIDetailInfos( float xPos, float yPos, float w, float h, UIMapViewGUI *setGui )
: UIWindow( xPos, yPos, w, h )
{
  this->mainGui = setGui;

  this->addChild( static_cast<UIFrame*>( new UIMinimizeButton( w, this ) ) );

  this->theInfos = new UIText( 8.0f, 7.0f, "", arial14, eJustifyLeft );
  this->addChild( this->theInfos );
}

void UIDetailInfos::setText( const std::string& t )
{
  this->theInfos->setText( t );
}
