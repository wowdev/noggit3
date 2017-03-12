// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>
#include <noggit/MapChunk.h>
#include <noggit/Selection.h>

class UITexture;

namespace noggit
{
  namespace ui
  {
    class current_texture;
  }
}

class UITexturePicker : public UICloseWindow
{
public:
  UITexturePicker ( float x, float y, float w, float h
                  , noggit::ui::current_texture*
                  );

  void getTextures(selection_type lSelection);
  void setTexture(size_t id, noggit::ui::current_texture*);
  void shiftSelectedTextureLeft();
  void shiftSelectedTextureRight();

private:
  void update();

  UITexture* _textures[4];
  MapChunk* _chunk;
  selection_type _select;
};
