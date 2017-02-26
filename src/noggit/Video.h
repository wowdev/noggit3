// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <math/vector_4d.hpp>
#include <opengl/types.hpp>

class Video
{
public:
  void init(int xres_, int yres_);

  void set2D() const;
  void setTileMode() const;
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
    return _ratio;
  }
  float nearclip() const
  {
    return _nearclip;
  }
  float farclip() const
  {
    return _farclip;
  }

  void nearclip(float nearclip_)
  {
    _nearclip = nearclip_;
  }
  void farclip(float farclip_)
  {
    _farclip = farclip_;
  }

  math::vector_4d normalized_device_coords (int x, int y) const;

private:
  int _xres;
  int _yres;
  float _ratio;

  float _nearclip;
  float _farclip;
};

extern Video video;
