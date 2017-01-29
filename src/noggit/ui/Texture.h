// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <functional>
#include <string>

#include <noggit/TextureManager.h>
#include <noggit/ui/Frame.h>

class UITexture : public UIFrame
{
public:
  typedef UITexture* Ptr;

protected:
  scoped_blp_texture_reference texture;
  std::string _textureFilename;

  bool highlight;
  std::function<void()> clickFunc;
  int id;

public:
  UITexture(float x, float y, float width, float height, const std::string& tex);

  void setTexture(const std::string& tex);
  void setTexture(scoped_blp_texture_reference const& tex);
  void render() const;

  UIFrame *processLeftClick(float mx, float my);
  void setClickFunc(std::function<void()>);
  void setHighlight(bool h)
  {
    highlight = h;
  }
  scoped_blp_texture_reference const& getTexture();
};
