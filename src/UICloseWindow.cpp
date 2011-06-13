#include "UICloseWindow.h"

#include <string>

#include "Noggit.h" // arial16
#include "UIMinimizeButton.h"
#include "UIText.h"

UICloseWindow::UICloseWindow( float px, float py, float w, float h, const std::string& pTitle, bool pMoveable )
: UIWindow( px, py, w, h )
{
  this->addChild( new UIText( w / 2.0f, 2.0f, pTitle, arial16, eJustifyCenter ) );
  this->addChild( new UIMinimizeButton( w, this ) );
  this->movable = pMoveable;
}
