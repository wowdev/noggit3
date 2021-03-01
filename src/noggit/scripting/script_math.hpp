// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <math/vector_3d.hpp>

#include <memory>
#include <string>

namespace noggit
{
  namespace scripting
  {
    float round(float a1);
    float pow(float a1, float a2);
    float log10(float arg);
    float log(float arg);
    float ceil(float arg);
    float floor(float arg);
    float exp(float arg);
    float cbrt(float arg);
    float acosh(float arg);
    float asinh(float arg);
    float atanh(float arg);
    float cosh(float arg);
    float sinh(float arg);
    float tanh(float arg);
    float acos(float arg);
    float asin(float arg);
    float atan(float arg);
    float cos(float arg);
    float sin(float arg);
    float tan(float arg);
    float sqrt(float arg);
    float abs(float arg);
    float lerp(float from, float to, float amount);
    float dist_2d(math::vector_3d& from, math::vector_3d& to);
    int dist_2d_compare(math::vector_3d& from, math::vector_3d& to, float dist);
    math::vector_3d rotate_2d(math::vector_3d& point, math::vector_3d& origin, float angleDeg);
  } // namespace scripting
} // namespace noggit
