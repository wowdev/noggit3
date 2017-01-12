#ifndef __TEXTUREPICKER_H
#define __TEXTUREPICKER_H

#include "UICloseWindow.h"
#include "MapChunk.h"
#include "Selection.h"

class UITexture;

class UITexturePicker : public UICloseWindow
{
public:
	UITexturePicker(float x, float y, float w, float h);

	void getTextures(selection_type lSelection);
	void setTexture(size_t id);
  void shiftSelectedTextureLeft();
  void shiftSelectedTextureRight();

private:
  void update();

	UITexture* _textures[4];
  MapChunk* _chunk;
  selection_type _select;
};

#endif
