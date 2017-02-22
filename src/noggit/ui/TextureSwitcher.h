// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>
#include <noggit/ui/Texture.h>

class UITextureSwitcher : public UICloseWindow
{
public:
  UITextureSwitcher (float x_right, float y, UIWindow* parent, const math::vector_3d* camera_pos);

  scoped_blp_texture_reference const& current_texture() const
  {
    return _textureFrom->getTexture();
  }

private:
  UITexture* _textureFrom;
};
