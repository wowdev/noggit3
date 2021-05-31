// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/constants.hpp>
#include <math/vector_3d.hpp>

#include <cmath>

namespace math
{
  namespace {
    inline float normalize_degrees(float deg) {
      return deg - std::floor((deg + 180.f) / 360.f) * 360.f;
    }
  }

  struct radians;

  struct degrees
  {
    explicit degrees (float x) : _ (normalize_degrees(x)) {}
    degrees (radians);

    float _;

    inline degrees operator+ (const degrees &v) const
    {
      return degrees (_ + v._);
    }

    inline degrees operator- (const degrees &v) const
    {
      return degrees (_ - v._);
    }

    inline degrees operator-() const
    {
      return degrees (-_);
    }

    inline degrees& operator+= (const degrees &v)
    {
      return *this = *this + v;
    }

    inline degrees& operator-= (const degrees &v)
    {
      return *this = *this - v;
    }

    friend std::ostream& operator<< (std::ostream& os, degrees const& v)
    {
      return os << v._ << "°";
    }

    explicit operator float() const { return _; }

    using vec3 = vector_3d_base<degrees>;
  };

  struct radians
  {
    explicit radians (float x) : _ (x) {}
    radians (degrees);

    float _;

    using vec3 = vector_3d_base<radians>;
  };

  inline degrees::degrees (radians x) : _ (x._ * 180.0f / math::constants::pi) {}
  inline radians::radians (degrees x) : _ (x._ * math::constants::pi / 180.0f) {}

  inline float sin (radians x)
  {
    return std::sin (x._);
  }
  inline float cos (radians x)
  {
    return std::cos (x._);
  }
  inline float tan (radians x)
  {
    return std::tan (x._);
  }
  inline radians asin (float x)
  {
    return radians {std::asin (x)};
  }
  inline radians acos (float x)
  {
    return radians {std::acos (x)};
  }
  inline radians atan2 (float y, float x)
  {
    return radians {std::atan2 (y, x)};
  }
}

inline math::degrees operator"" _deg (long double v)
{
  return math::degrees {static_cast<float> (v)};
}

inline math::degrees operator"" _deg (unsigned long long int v)
{
  return math::degrees {static_cast<float> (v)};
}
