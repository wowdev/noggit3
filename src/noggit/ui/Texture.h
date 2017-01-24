// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <functional>
#include <string>

#include <noggit/ui/Frame.h>

namespace OpenGL
{
  class Texture;
}

class UITexture : public UIFrame
{
public:
  typedef UITexture* Ptr;

protected:
  OpenGL::Texture* texture;
  std::string _textureFilename;

  bool highlight;
  std::function<void()> clickFunc;
  int id;

public:
  UITexture(float x, float y, float width, float height, const std::string& tex);
  ~UITexture();

  void setTexture(const std::string& tex);
  void setTexture(OpenGL::Texture* tex);
  void render() const;

  UIFrame *processLeftClick(float mx, float my);
  void setClickFunc(std::function<void()>);
  void setHighlight(bool h)
  {
    highlight = h;
  }
  OpenGL::Texture* getTexture();
};
