// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "UIWindow.h"

class UITexture;
class UIButton;

class UITextureSwitcher : public UIWindow
{
public:
	UITextureSwitcher(int x, int y);

	OpenGL::Texture* getTextures();
	void setTexture();
	void setPosition(float x, float y);

private:
	UITexture *_textureFrom;
	UITexture *_textureTo;

	UIButton *_setFromButton;
	UIButton *_setToButton;
	float xPos, zPos;
};
