#include <noggit/UICloseWindow.h>

#include <string>

#include <noggit/application.h> // arial16
#include <noggit/UIMinimizeButton.h>
#include <noggit/UIText.h>

UICloseWindow::UICloseWindow( float px, float py, float w, float h, const std::string& pTitle, bool pMoveable )
: UIWindow( px, py, w, h )
{
  addChild( new UIText( width() / 2.0f, 2.0f, pTitle, arial16, eJustifyCenter ) );
  addChild( new UIMinimizeButton( width() ) );
  movable( pMoveable );
}
