// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>

#include <ostream>
#include <tuple>

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

    vector_2d() : vector_2d (0.f, 0.f) {}
    vector_2d (float x_, float y_)
      : x (x_)
      , y (y_)
    {}

    inline operator float*()
    {
      return _data;
    }
    inline operator float const*() const
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

    bool operator== (vector_2d const& rhs) const
    {
      return std::tie (x, y) == std::tie (rhs.x, rhs.y);
    }
    friend std::ostream& operator<< (std::ostream& os, vector_2d const& x)
    {
      return os << x.x << ", " << x.y;
    }
  };

  void rotate (float x0, float y0, float* x, float* y, radians);
  inline vector_2d rotate (vector_2d const& around, vector_2d point, radians angle)
  {
    rotate (around.x, around.y, &point.x, &point.y, angle);
    return point;
  }
}
