// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>

namespace math
{
  struct vector_2d
  {
    union
    {
      float _data[2];
      struct
      {
        float x;
        float y;
      };
    };

    vector_2d (float x_ = 0.0f, float y_ = 0.0f)
      : x (x_)
      , y (y_)
    {}

    inline operator float*()
    {
      return _data;
    }

    vector_2d operator* (float factor) const
    {
      return {x * factor, y * factor};
    }
    vector_2d operator+ (vector_2d const& other) const
    {
      return {x + other.x, y + other.y};
    }
  };

  void rotate (float x0, float y0, float* x, float* y, radians);
}
