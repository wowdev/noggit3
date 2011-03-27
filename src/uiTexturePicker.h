#ifndef __TEXTUREPICKER_H
#define __TEXTUREPICKER_H

#include "window.h"
#include "buttonUI.h"
#include "textureUI.h"
#include "selection.h"
class Gui;

class uiTexturePicker : public window
{
public:
	uiTexturePicker(int xPos,int yPos, int w, int h, Gui *setGui);
	void getTextures(nameEntry *lSelection);
	void setTexture(int id);
	Gui *mainGUI;
private:
	textureUI *tex1;
	textureUI *tex2;
	textureUI *tex3;
	textureUI *tex4;

};

#endif