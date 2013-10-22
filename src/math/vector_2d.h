// vector_2d.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __MATH_VECTOR_2D_H
#define __MATH_VECTOR_2D_H

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

#endif
