// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>

class UITexture;
class UIButton;

class UITextureSwitcher : public UIWindow
{
public:
  UITextureSwitcher(int x, int y);

  scoped_blp_texture_reference const& getTextures();
  void setTexture();
  void setPosition(float x, float y);

private:
  UITexture *_textureFrom;

  UIButton *_setFromButton;
};
