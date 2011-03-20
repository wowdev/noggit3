#include "noggit.h" // arial16

#include "closeWindowUI.h"

#include "MinimizeButton.h"
#include "textUI.h"


closeWindowUI::closeWindowUI( float px, float py, float w, float h, const std::string& pTitle, bool pMoveable ) : window( px, py, w, h )
{
	this->addChild( new textUI( w / 2.0f, 2.0f, pTitle, arial16, eJustifyCenter ) );
	this->addChild( new MinimizeButton( w, this ) );
	this->movable = pMoveable;
}
