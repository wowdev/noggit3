#ifndef __TEXTUREPICKER_H
#define __TEXTUREPICKER_H

#include "UICloseWindow.h"

class nameEntry;
class UITexture;

class UITexturePicker : public UICloseWindow
{
public:
	UITexturePicker(float x, float y, float w, float h);

	void getTextures(nameEntry* lSelection);
	void setTexture(size_t id);

private:
	UITexture* _textures[4];
};

#endif
