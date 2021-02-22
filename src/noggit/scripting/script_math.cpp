// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_math.hpp>
#include <cmath>

using namespace noggit::scripting;

float noggit::scripting::round(float a1) { return ::round(a1); }
float noggit::scripting::pow(float a1, float a2) { return ::pow(a1, a2); }
float noggit::scripting::log10(float arg) { return ::log10(arg); }
float noggit::scripting::log(float arg) { return ::log(arg); }
float noggit::scripting::ceil(float arg) { return (int)::ceil(arg); }
float noggit::scripting::floor(float arg) { return (int)::floor(arg); }
float noggit::scripting::exp(float arg) { return ::exp(arg); }
float noggit::scripting::cbrt(float arg) { return ::cbrt(arg); }
float noggit::scripting::acosh(float arg) { return ::acosh(arg); }
float noggit::scripting::asinh(float arg) { return ::asinh(arg); }
float noggit::scripting::atanh(float arg) { return ::atanh(arg); }
float noggit::scripting::cosh(float arg) { return ::cosh(arg); }
float noggit::scripting::sinh(float arg) { return ::sinh(arg); }
float noggit::scripting::tanh(float arg) { return ::tanh(arg); }
float noggit::scripting::acos(float arg) { return ::acos(arg); }
float noggit::scripting::asin(float arg) { return ::asin(arg); }
float noggit::scripting::atan(float arg) { return ::atan(arg); }
float noggit::scripting::cos(float arg) { return ::cos(arg); }
float noggit::scripting::sin(float arg) { return ::sin(arg); }
float noggit::scripting::tan(float arg) { return ::tan(arg); }
float noggit::scripting::sqrt(float arg) { return ::sqrt(arg); }
float noggit::scripting::abs(float arg) { return ::abs(arg); }
float noggit::scripting::lerp(float from, float to, float ratio) { return from + ratio * (to - from); }
float noggit::scripting::dist_2d(math::vector_3d &from, math::vector_3d &to)
{
    return std::sqrt(std::pow(from.x - to.x, 2) + std::pow(from.z - to.z, 2));
}

int noggit::scripting::dist_2d_compare(math::vector_3d &from, math::vector_3d &to, float compare)
{
    float dist = std::pow(from.x - to.x, 2) + std::pow(from.z - to.z, 2);
    compare = std::pow(compare, 2);
    return dist > compare ? 1 : dist == compare ? 0 : -1;
}

math::vector_3d noggit::scripting::rotate_2d(math::vector_3d &point, math::vector_3d &origin, float angle)
{
    float s = std::sin(angle * 0.0174532925);
    float c = std::cos(angle * 0.0174532925);

    float lx = point.x - origin.x;
    float lz = point.z - origin.z;

    float nx = lx * c - lz * s + origin.x;
    float nz = lx * s + lz * c + origin.z;

    return math::vector_3d(nx, point.y, nz);
}