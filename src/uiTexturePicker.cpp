#include <iostream>
#include <sstream>

#include "uiTexturePicker.h"
#include "window.h"
#include "textUI.h" // textUI
#include "noggit.h" // arial14, arialn13

uiTexturePicker::uiTexturePicker( int xPos,int yPos, int w, int h, Gui *setGui ) : window(xPos,yPos,w,h)
{
	this->tex1 = new textureUI(10,30,110,110,"tileset\\generic\\black.blp");
	this->tex2 = new textureUI(130,30,110,110,"tileset\\generic\\black.blp");
	this->tex3 = new textureUI(250,30,110,110,"tileset\\generic\\black.blp");
	this->tex4 = new textureUI(370,30,110,110,"tileset\\generic\\black.blp");
	this->addChild(this->tex1);
	this->addChild(this->tex2);
	this->addChild(this->tex3);
	this->addChild(this->tex4);
	this->addChild(new textUI( 10.0f, 9.0f, "Pick one of the textures.", arialn13, eJustifyLeft ) );
}
