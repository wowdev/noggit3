// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>
#include <boost/function.hpp>

#include <noggit/TextureManager.h>
#include <noggit/ui/Frame.h>

class UISlider : public UIFrame
{
protected:
  scoped_blp_texture_reference texture;
  scoped_blp_texture_reference sliderTexture;
  float scale;
  float offset;
  std::function<void(float)> func;
  std::string text;

public:
  float value;
  void setFunc(void(*f)(float value));
  void setFunc(std::function<void(float value)> pFunc);
  void setValue(float f);
  void setText(const std::string& text);
  UISlider(float x, float y, float width, float s, float o);
  UIFrame* processLeftClick(float mx, float my);
  bool processLeftDrag(float mx, float my, float xChange, float yChange);
  void render() const;
};
