// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Frame.h>

#include <functional>
#include <string>

namespace OpenGL { class Texture; };

class UIToolbarIcon : public UIFrame
{
protected:
  OpenGL::Texture* texture;
  OpenGL::Texture* textureSelected;

  std::string _textureFilename;
  std::string _textureSelectedFilename;

  std::function<void()> _callback;

public:
  UIToolbarIcon(float x, float y, const std::string& tex, const std::string& texd, std::function<void()>);
  ~UIToolbarIcon();

  void render() const;
  UIFrame *processLeftClick(float mx, float my);

  bool selected;
};
