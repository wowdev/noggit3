#include <iostream>
#include <sstream>

#include "uiTexturePicker.h"
#include "Gui.h"
#include "window.h"
#include "textUI.h" // textUI
#include "noggit.h" // arial14, arialn13
#include "MinimizeButton.h"
#include "MapChunk.h"
#include "selection.h"
#include "Toolbar.h"
#include "textureUI.h"

void textureClick( frame* f,int id )
{
	// Implement texture set.
}

uiTexturePicker::uiTexturePicker( int xPos,int yPos, int w, int h, Gui *setGui ) : window(xPos,yPos,w,h)
{
	this->mainGUI = setGui;

	this->tex1 = new textureUI(10,30,110,110,"tileset\\generic\\black.blp");
	this->tex2 = new textureUI(130,30,110,110,"tileset\\generic\\black.blp");
	this->tex3 = new textureUI(250,30,110,110,"tileset\\generic\\black.blp");
	this->tex4 = new textureUI(370,30,110,110,"tileset\\generic\\black.blp");

	this->tex1->setClickFunc(textureClick,0);
	this->tex2->setClickFunc(textureClick,1);
	this->tex3->setClickFunc(textureClick,2);
	this->tex4->setClickFunc(textureClick,3);

	this->addChild(this->tex1);
	this->addChild(this->tex2);
	this->addChild(this->tex3);
	this->addChild(this->tex4);
	this->addChild(new textUI( 10.0f, 9.0f, "Pick one of the textures.", arial14, eJustifyLeft ) );

	//close button
	this->addChild( new MinimizeButton( w, this ) );

}

void uiTexturePicker::setTextures(nameEntry *lSelection)
{
	this->hidden = false;
	if( lSelection )
		if( lSelection->type == eEntry_MapChunk )
		{
				if(lSelection->data.mapchunk->nTextures > 0)
				{
					this->tex1->setTexture(lSelection->data.mapchunk->textures[0]);
					this->tex1->hidden = false;
				}
				else this->tex1->hidden = true;
				if(lSelection->data.mapchunk->nTextures > 1)				
				{
					this->tex2->setTexture(lSelection->data.mapchunk->textures[1]);
					this->tex2->hidden = false;
				}
				else this->tex2->hidden = true;
				if(lSelection->data.mapchunk->nTextures > 2)				
				{
					this->tex3->setTexture(lSelection->data.mapchunk->textures[2]);
					this->tex3->hidden = false;
				}
				else this->tex3->hidden = true;
				if(lSelection->data.mapchunk->nTextures > 3)
				{
					this->tex4->setTexture(lSelection->data.mapchunk->textures[3]);
					this->tex4->hidden = false;
				}
				else this->tex4->hidden = true;
		}
}

