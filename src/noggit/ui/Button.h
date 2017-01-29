// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>
#include <boost/function.hpp>

#include <noggit/TextureManager.h>
#include <noggit/ui/Frame.h>
#include <noggit/ui/Text.h>

namespace OpenGL { class Texture; };
namespace freetype { class font_data; };

class UIButton : public UIFrame
{
public:
  typedef UIButton* Ptr;
  typedef void(*ClickFunction)(UIFrame::Ptr, int);

protected:
  scoped_blp_texture_reference texture;
  scoped_blp_texture_reference textureDown;

  std::function<void()> clickFunc;
  int id;

  bool clicked;
  UIText::Ptr text;

public:
  explicit UIButton(float x, float y, float width, float height, const std::string& pTexNormal, const std::string& pTexDown);
  explicit UIButton(float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown);
  explicit UIButton(float x, float y, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown);
  explicit UIButton(float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown, ClickFunction pFunc, int pFuncParam);
  explicit UIButton(float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown, std::function<void()>);

  void render() const;

  void setLeft();
  void setText(const std::string& pText);

  UIFrame::Ptr processLeftClick(float mx, float my);
  void setClickFunc (std::function<void()>);
  void processUnclick();
};
