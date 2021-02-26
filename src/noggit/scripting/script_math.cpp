// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_math.hpp>

#include <cmath>

namespace noggit
{
  namespace scripting
  {
    float round(float a1) { return ::round(a1); }
    float pow(float a1, float a2) { return ::pow(a1, a2); }
    float log10(float arg) { return ::log10(arg); }
    float log(float arg) { return ::log(arg); }
    float ceil(float arg) { return (int)::ceil(arg); }
    float floor(float arg) { return (int)::floor(arg); }
    float exp(float arg) { return ::exp(arg); }
    float cbrt(float arg) { return ::cbrt(arg); }
    float acosh(float arg) { return ::acosh(arg); }
    float asinh(float arg) { return ::asinh(arg); }
    float atanh(float arg) { return ::atanh(arg); }
    float cosh(float arg) { return ::cosh(arg); }
    float sinh(float arg) { return ::sinh(arg); }
    float tanh(float arg) { return ::tanh(arg); }
    float acos(float arg) { return ::acos(arg); }
    float asin(float arg) { return ::asin(arg); }
    float atan(float arg) { return ::atan(arg); }
    float cos(float arg) { return ::cos(arg); }
    float sin(float arg) { return ::sin(arg); }
    float tan(float arg) { return ::tan(arg); }
    float sqrt(float arg) { return ::sqrt(arg); }
    float abs(float arg) { return ::abs(arg); }
    float lerp(float from, float to, float ratio) { return from + ratio * (to - from); }
    float dist_2d(math::vector_3d &from, math::vector_3d &to)
    {
      return std::sqrt(std::pow(from.x - to.x, 2) + std::pow(from.z - to.z, 2));
    }

    int dist_2d_compare(math::vector_3d &from, math::vector_3d &to, float compare)
    {
      float dist = std::pow(from.x - to.x, 2) + std::pow(from.z - to.z, 2);
      compare = std::pow(compare, 2);
      return dist > compare ? 1 : dist == compare ? 0 : -1;
    }

    math::vector_3d rotate_2d(math::vector_3d &point, math::vector_3d &origin, float angle)
    {
      float s = std::sin(angle * 0.0174532925);
      float c = std::cos(angle * 0.0174532925);

      float lx = point.x - origin.x;
      float lz = point.z - origin.z;

      float nx = lx * c - lz * s + origin.x;
      float nz = lx * s + lz * c + origin.z;

      return math::vector_3d(nx, point.y, nz);
    }
  } // namespace scripting
} // namespace noggit