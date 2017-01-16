// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>
#include <noggit/MapChunk.h>
#include <noggit/Selection.h>

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
