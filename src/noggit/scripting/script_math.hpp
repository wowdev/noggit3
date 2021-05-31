// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <math/vector_3d.hpp>

#include <memory>
#include <string>

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class script_context;
    int round(float a1);
    float pow(float a1, float a2);
    float log10(float arg);
    float log(float arg);
    int ceil(float arg);
    int floor(float arg);
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
    float dist_2d(math::vector_3d const& from, math::vector_3d const& to);
    int dist_2d_compare(math::vector_3d const& from, math::vector_3d const& to, float dist);
    math::vector_3d rotate_2d(math::vector_3d const& point, math::vector_3d const& origin, float angleDeg);

    void register_math(script_context * state);
  } // namespace scripting
} // namespace noggit
