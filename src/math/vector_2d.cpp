// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/vector_2d.h>

#include <cmath>

namespace math
{
  void rotate (float x0, float y0, float* x, float* y, float radians)
  {
    static const float radians_to_angle_constant (constants::pi() / 180.0f);
    const float angle (radians * radians_to_angle_constant);
    const float xa (*x - x0);
    const float ya (*y - y0);
    *x = xa * cosf (angle) - ya * sinf (angle) + x0;
    *y = xa * sinf (angle) + ya * cosf (angle) + y0;
  }
}
