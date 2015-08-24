#include "UITextureSwitcher.h"

#include "Noggit.h" // app.getArial16()
#include "World.h"

#include "UITexture.h"
#include "UITexturingGUI.h"
#include "UIButton.h"
#include "UIText.h"
#include "MapView.h"
#include "Misc.h"
#include "TextureSet.h"

extern UIWindow *settings_paint;
extern UIWindow *settings_paint;
extern World *gWorld;

void swapADT(UIFrame *f, int id)
{
	gWorld->swapTexture(misc::FtoIround((gWorld->camera.x - (TILESIZE / 2)) / TILESIZE), misc::FtoIround((gWorld->camera.z - (TILESIZE / 2)) / TILESIZE), (static_cast<UITextureSwitcher *>(f->parent()))->getTextures());
}


void setTexturePressed(UIFrame *f, int id)
{
	(static_cast<UITextureSwitcher *>(f->parent()))->setTexture();
}

void closeButtonPressed(UIFrame *f, int id)
{
	settings_paint->show();
	if (f->parent())
		f->parent()->hide();
}

UITextureSwitcher::UITextureSwitcher(int x, int y)
	: UIWindow((float)x, (float)y, 130, 200)
{
	const int textureSize = 110;

	_textureFrom = new UITexture(10, 60, (float)textureSize, (float)textureSize, "tileset\\generic\\black.blp");
	_setFromButton = new UIButton(10, 65 + (float)textureSize, (float)textureSize, 30, "set destination", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", setTexturePressed, 0);

	addChild(_textureFrom);
	addChild(_setFromButton);

	addChild(new UIText(width() / 2.0f, 2.0f, "swapper", app.getArial16(), eJustifyCenter));
	addChild(new UIButton(width() - 29.0f, 1.0f, 30.0f, 30.0f, "", "Interface\\Buttons\\UI-Panel-MinimizeButton-Up.blp", "Interface\\Buttons\\UI-Panel-MinimizeButton-Down.blp", closeButtonPressed, 0));
	addChild(new UIButton(10, 30, (float)textureSize, 30, "swap ADT", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", swapADT, 0));
}

OpenGL::Texture*  UITextureSwitcher::getTextures()
{
	return _textureFrom->getTexture();
}

void UITextureSwitcher::setTexture()
{
	_textureFrom->setTexture(UITexturingGUI::getSelectedTexture());
}

void UITextureSwitcher::setPosition(float setX, float setY)
{
	this->_x = setX;
	this->_y = setY;
}



