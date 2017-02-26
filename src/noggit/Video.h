// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <math/vector_4d.hpp>
#include <opengl/types.hpp>

class Video
{
public:
  void init(int xres_, int yres_);

  void resize(int w, int h);

  int xres() const
  {
    return _xres;
  }
  int yres() const
  {
    return _yres;
  }
  float ratio() const
  {
    return static_cast<float>(xres()) / static_cast<float>(yres());
  }

  float farclip() const
  {
    return _farclip;
  }

  void farclip(float farclip_)
  {
    _farclip = farclip_;
  }

private:
  int _xres;
  int _yres;
  float _ratio;

  float _farclip;
};

extern Video video;
