#include "UITexturePicker.h"

#include "Selection.h"
#include "TextureSet.h"
#include "UITexture.h"
#include "UITexturingGUI.h"
#include "UIButton.h"
#include "World.h"

#include <cassert>

void texturePickerClick(UIFrame* f, int id)
{
	// redirect to sender object.
	(static_cast<UITexturePicker *>(f->parent()))->setTexture(id);
}

void shiftLeft(UIFrame* f, int)
{
  (static_cast<UITexturePicker *>(f->parent()))->shiftSelectedTextureLeft();
}

void shiftRight(UIFrame* f, int)
{
  (static_cast<UITexturePicker *>(f->parent()))->shiftSelectedTextureRight();
}

UITexturePicker::UITexturePicker(float x, float y, float w, float h)
	: UICloseWindow(x, y, w, h, "Pick one of the textures.", true)
{
	const int textureSize = 110;
	const int startingX = 10;
	const int paddingX = 10;
	const int positionY = 30;

  _chunk = nullptr;

	for (size_t i = 0; i < 4; ++i)
	{
		_textures[i] = new UITexture((float)(startingX + (textureSize + paddingX) * i), (float)positionY, (float)textureSize, (float)textureSize, "tileset\\generic\\black.blp");
		_textures[i]->setClickFunc(texturePickerClick, i);
		addChild(_textures[i]);
	}

  UIButton* bleft = new UIButton((w / 2 - 65), (h - 27.5), 60.0f, 30.0f, "<<<"
                                , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                                , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                                , shiftLeft, 1);
  UIButton* bright = new UIButton((w / 2 + 5), (h - 27.5), 60.0f, 30.0f, ">>>"
                                  , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                                  , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                                  , shiftRight, 1);
  addChild(bleft);
  addChild(bright);
}

void UITexturePicker::getTextures(nameEntry* lSelection)
{
	assert(lSelection);

	show();

	if (lSelection && lSelection->type == eEntry_MapChunk)
	{
		MapChunk* chunk = lSelection->data.mapchunk;
    _chunk = chunk;
		size_t index = 0;

		for (; index < 4U && chunk->textureSet->num() > index; ++index)
		{
			_textures[index]->setTexture(chunk->textureSet->texture(index));
			_textures[index]->show();
		}

		for (; index < 4U; ++index)
		{
			_textures[index]->hide();
		}
	}
}

void UITexturePicker::setTexture(size_t id)
{
	assert(id < 4);

	UITexturingGUI::setSelectedTexture(_textures[id]->getTexture());
	UITexturingGUI::updateSelectedTexture();
}

void UITexturePicker::shiftSelectedTextureLeft()
{
  OpenGL::Texture* selectedTexture = UITexturingGUI::getSelectedTexture();
  TextureSet* ts = _chunk->textureSet;
  for (int i = 1; i < ts->num(); i++)
  {
    if (ts->texture(i) == selectedTexture)
    {
      ts->swapTexture(i - 1, i);
      update();
      return;
    }
  }
}

void UITexturePicker::shiftSelectedTextureRight()
{
  OpenGL::Texture* selectedTexture = UITexturingGUI::getSelectedTexture();
  TextureSet* ts = _chunk->textureSet;
  for (int i = 0; i < ts->num() - 1; i++)
  {
    if (ts->texture(i) == selectedTexture)
    {
      ts->swapTexture(i, i + 1);
      update();
      return;
    }
  }
}

void UITexturePicker::update()
{
  _chunk->mt->changed = true;

  for (size_t index = 0; index < 4U && _chunk->textureSet->num() > index; ++index)
  {
    _textures[index]->setTexture(_chunk->textureSet->texture(index));
    _textures[index]->show();
  }
}