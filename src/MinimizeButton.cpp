#include "MinimizeButton.h"

MinimizeButton::MinimizeButton( float pWidth, frame * pParent ) : buttonUI(
	pWidth - 29.0f, 1.0f,
	30.0f, 30.0f, 
	video.textures.add( "Interface\\Buttons\\UI-Panel-MinimizeButton-Up.blp" ), 
	video.textures.add( "Interface\\Buttons\\UI-Panel-MinimizeButton-Down.blp" )
	)
{
	mParent = pParent;
}

frame *MinimizeButton::processLeftClick(float mx,float my)
{
	clicked = true;
	if( mParent )
		mParent->hidden = true;
	return this;
}
