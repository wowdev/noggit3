// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.hpp> // math::vector_4d
#include <noggit/ui/Frame.h> // UIFrame

#include <functional>

class UIGradient : public UIFrame
{
protected:
  math::vector_4d  MinColor;
  math::vector_4d  MaxColor;
  math::vector_4d  ClickColor;
  float  value;
  std::function<void(float)> func;

public:
  bool  horiz;

  UIGradient(float x, float y, float width, float height, bool horizontal);

  void  setValue(float f);
  void  setClickFunc(std::function<void(float)> f);
  void  setMinColor(float r, float g, float b, float a);
  void  setMaxColor(float r, float g, float b, float a);
  void  setClickColor(float r, float g, float b, float a);
  UIFrame  *processLeftClick(float mx, float my);
  bool  processLeftDrag(float mx, float my, float xChange, float yChange);
  void  render() const;
};
