// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UICloseWindow.h>

class nameEntry;
class UITexture;

class UITexturePicker : public UICloseWindow
{
public:
  UITexturePicker( float x, float y, float w, float h );

  void getTextures( nameEntry* lSelection );
  void setTexture( size_t id );

private:
  UITexture* _textures[4];
};
