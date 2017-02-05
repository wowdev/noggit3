// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>
#include <noggit/ui/Frame.h>

#include <functional>
#include <string>

class UIToolbarIcon : public UIFrame
{
protected:
  scoped_blp_texture_reference texture;
  scoped_blp_texture_reference textureSelected;

  std::function<void()> _callback;

public:
  UIToolbarIcon(float x, float y, const std::string& tex, const std::string& texd, std::function<void()>);

  void render() const;
  UIFrame *processLeftClick(float mx, float my);

  bool selected;
};
