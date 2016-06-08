// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/constants.h>

namespace math
{
  class vector_2d
  {
  public:
    vector_2d (float x = 0.0f, float y = 0.0f)
    : _x (x)
    , _y (y)
    { }

    inline const float& x() const
    {
      return _x;
    }
    inline const float& y() const
    {
      return _y;
    }

    inline operator float*()
    {
      return _data;
    }

    vector_2d operator* (float factor) const
    {
      return {_x * factor, _y * factor};
    }
    vector_2d operator+ (vector_2d const& other) const
    {
      return {_x + other._x, _y + other._y};
    }

  private:
    union
    {
      struct
      {
        float _x;
        float _y;
      };
      float _data[2];
    };
  };

  void rotate (float x0, float y0, float* x, float* y, float radians);
}
