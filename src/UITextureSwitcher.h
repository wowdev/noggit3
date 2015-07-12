#ifndef __TEXTURESWITCHER_H
#define __TEXTURESWITCHER_H

#include "UIWindow.h"

class nameEntry;
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

#endif
