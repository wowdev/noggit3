#include <noggit/UIMinimizeButton.h>

#include <string>

UIMinimizeButton::UIMinimizeButton( float pWidth )
: UIButton( pWidth - 29.0f, 1.0f, 30.0f, 30.0f, "Interface\\Buttons\\UI-Panel-MinimizeButton-Up.blp", "Interface\\Buttons\\UI-Panel-MinimizeButton-Down.blp" )
{
}

UIFrame::Ptr UIMinimizeButton::processLeftClick( float /*mx*/, float /*my*/ )
{
  clicked = true;
  if( parent() )
    parent()->hide();
  return this;
}
