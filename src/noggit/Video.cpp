// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Settings.h>
#include <noggit/Video.h>
#include <opengl/matrix.hpp>

Video video;

math::vector_4d Video::normalized_device_coords (int x, int y) const
{
  return {2.0f * x / xres() - 1.0f, 1.0f - 2.0f * y / yres(), 0.0f, 1.0f};
}

void Video::resize(int xres_, int yres_)
{
  _xres = xres_;
  _yres = yres_;
  _ratio = static_cast<float>(xres()) / static_cast<float>(yres());

  gl.viewport(0.0f, 0.0f, xres(), yres());
}

void Video::init(int xres_, int yres_)
{
  resize (xres_, yres_);

  _nearclip = 1.0f;
  _farclip = Settings::getInstance()->FarZ;
}
