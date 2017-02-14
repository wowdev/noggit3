// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Gradient.h>

#include <noggit/Video.h> // gl*
#include <opengl/scoped.hpp>

#include <iostream>     // std::cout
#include <algorithm>    // std::min


UIGradient::UIGradient(float x, float y, float width, float height, bool horizontal)
  : UIFrame(x, y, width, height)
  , horiz(horizontal)
{
}

void UIGradient::render() const
{
  if (hidden())
  {
    // return;
  }

  opengl::scoped::matrix_pusher const matrix;
  gl.translatef(x(), y(), 0.0f);

  if (horiz)
  {
    gl.begin(GL_TRIANGLE_STRIP);
    gl.color4fv(&MinColor.x);
    gl.vertex2f(0.0f, 0.0f);
    gl.vertex2f(0.0f, height());
    gl.color4fv(&MaxColor.x);
    gl.vertex2f(width(), 0.0f);
    gl.vertex2f(width(), height());
    gl.end();

    if (clickable())
    {
      gl.color4fv(&ClickColor.x);
      gl.begin(GL_LINE);
      gl.vertex2f(width() * value, 0.0f);
      gl.vertex2f(width() * value, height());
      gl.end();
    }
  }
  else
  {
    gl.begin(GL_TRIANGLE_STRIP);
    gl.color4fv(&MinColor.x);
    gl.vertex2f(width(), 0.0f);
    gl.vertex2f(0.0f, 0.0f);
    gl.color4fv(&MaxColor.x);
    gl.vertex2f(width(), height());
    gl.vertex2f(0.0f, height());
    gl.end();



    if (clickable())
    {
      gl.begin(GL_TRIANGLE_STRIP);
      gl.color4fv(&ClickColor.x);
      gl.vertex2f(width(), (height() * value));
      gl.vertex2f(0.0f, (height() * value));
      gl.vertex2f(width(), (height() * value - 1.5f));
      gl.vertex2f(0.0f, (height() * value - 1.5f));
      gl.end();
    }
  }
}

void UIGradient::setMaxColor(float r, float g, float b, float a)
{
  MaxColor = math::vector_4d(r, g, b, a);
}

void UIGradient::setMinColor(float r, float g, float b, float a)
{
  MinColor = math::vector_4d(r, g, b, a);
}

void UIGradient::setClickColor(float r, float g, float b, float a)
{
  ClickColor = math::vector_4d(r, g, b, a);
}

void UIGradient::setClickFunc(std::function<void(float)> f)
{
  value = 0.0f;
  func = f;
  clickable(true);
}

UIFrame::Ptr UIGradient::processLeftClick(float mx, float my)
{
  if (clickable())
  {
    if (horiz)
      value = mx / width();
    else
      value = my / height();

    value = std::min(std::max(value, 0.0f), 1.0f);

    func(value);
    return this;
  }

  return nullptr;
}

bool UIGradient::processLeftDrag(float mx, float my, float xDrag, float yDrag)
{
  float tx(0.0f);
  float ty(0.0f);

  parent()->getOffset(&tx, &ty);

  mx -= tx;
  my -= ty;

  if (processLeftClick(mx, my))
  {
    return true;
  }

  return UIFrame::processLeftDrag(mx, my, xDrag, yDrag);
}

void UIGradient::setValue(float f)
{
  value = std::min(std::max(f, 0.0f), 1.0f);
  if (func)
  {
    func(value);
  }
}
