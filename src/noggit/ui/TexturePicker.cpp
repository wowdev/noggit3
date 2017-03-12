// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/TexturePicker.h>

#include <noggit/Selection.h>
#include <noggit/texture_set.hpp>
#include <noggit/ui/Texture.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/Button.h>
#include <noggit/World.h>

#include <cassert>

UITexturePicker::UITexturePicker
    ( float x, float y, float w, float h
    , noggit::ui::current_texture* current_texture_window
    )
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
    _textures[i]->setClickFunc
      ([=] { setTexture (i, current_texture_window); });
    addChild(_textures[i]);
  }

  UIButton* bleft = new UIButton((w / 2 - 65), (h - 27.5), 60.0f, 30.0f, "<<<"
                                , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                                , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                                , [this] { shiftSelectedTextureLeft(); });
  UIButton* bright = new UIButton((w / 2 + 5), (h - 27.5), 60.0f, 30.0f, ">>>"
                                  , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                                  , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                                 , [this] { shiftSelectedTextureRight(); });
  addChild(bleft);
  addChild(bright);
}

void UITexturePicker::getTextures(selection_type lSelection)
{
  show();

  if (lSelection.which() == eEntry_MapChunk)
  {
    MapChunk* chunk = boost::get<selected_chunk_type> (lSelection).chunk;
    _chunk = chunk;
    size_t index = 0;

    for (; index < 4U && chunk->_texture_set.num() > index; ++index)
    {
      _textures[index]->setTexture(chunk->_texture_set.texture(index));
      _textures[index]->show();
    }

    for (; index < 4U; ++index)
    {
      _textures[index]->hide();
    }
  }
}

void UITexturePicker::setTexture
  (size_t id, noggit::ui::current_texture* current_texture_window)
{
  assert(id < 4);

  UITexturingGUI::setSelectedTexture(_textures[id]->getTexture());
  UITexturingGUI::updateSelectedTexture (current_texture_window);
}

void UITexturePicker::shiftSelectedTextureLeft()
{
  auto&& selectedTexture = UITexturingGUI::getSelectedTexture();
  TextureSet& ts = _chunk->_texture_set;
  for (int i = 1; i < ts.num(); i++)
  {
    if (ts.texture(i) == selectedTexture)
    {
      ts.swapTexture(i - 1, i);
      update();
      return;
    }
  }
}

void UITexturePicker::shiftSelectedTextureRight()
{
  auto&& selectedTexture = UITexturingGUI::getSelectedTexture();
  TextureSet& ts = _chunk->_texture_set;
  for (int i = 0; i < ts.num() - 1; i++)
  {
    if (ts.texture(i) == selectedTexture)
    {
      ts.swapTexture(i, i + 1);
      update();
      return;
    }
  }
}

void UITexturePicker::update()
{
  _chunk->mt->changed = true;

  for (size_t index = 0; index < 4U && _chunk->_texture_set.num() > index; ++index)
  {
    _textures[index]->setTexture(_chunk->_texture_set.texture(index));
    _textures[index]->show();
  }
}
