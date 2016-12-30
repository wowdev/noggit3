// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace math
{
  struct vector_3d
  {
    union
    {
      float _data[3];
      struct
      {
        float x;
        float y;
        float z;
      };
    };

    vector_3d (float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f)
      : x (x_)
      , y (y_)
      , z (z_)
    {}

    inline static vector_3d min()
    {
      return {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    }
    inline static vector_3d max()
    {
      return {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    }

    inline vector_3d& operator= (const vector_3d &v)
    {
      x = v.x;
      y = v.y;
      z = v.z;
      return *this;
    }

    inline vector_3d operator+ (const vector_3d &v) const
    {
      return vector_3d (x + v.x, y + v.y, z + v.z);
    }

    inline vector_3d operator- (const vector_3d &v) const
    {
      return vector_3d (x - v.x, y - v.y, z - v.z);
    }

    inline vector_3d operator-() const
    {
      return vector_3d (-x, -y, -z);
    }

    inline float operator* (const vector_3d &v) const
    {
      return x * v.x + y * v.y + z * v.z;
    }

    inline vector_3d operator* (const float& d) const
    {
      return vector_3d (x * d, y * d, z * d);
    }

    friend vector_3d operator* (const float& d, const vector_3d& v)
    {
      return v * d;
    }

    inline vector_3d operator% (const vector_3d& v) const
    {
      return vector_3d ( y * v.z - z * v.y
                       , z * v.x - x * v.z
                       , x * v.y - y * v.x
                       );
    }

    inline vector_3d& operator+= (const vector_3d& v)
    {
      x += v.x;
      y += v.y;
      z += v.z;
      return *this;
    }

    inline vector_3d& operator-= (const vector_3d& v)
    {
      x -= v.x;
      y -= v.y;
      z -= v.z;
      return *this;
    }

    inline vector_3d& operator*= (float d)
    {
      x *= d;
      y *= d;
      z *= d;
      return *this;
    }

    inline float length_squared() const
    {
      return x * x + y * y + z * z;
    }

    inline float length() const
    {
      return std::sqrt (length_squared());
    }

    inline vector_3d& normalize()
    {
      return operator *= (1.0f / length());
    }

    vector_3d normalized() const
    {
      return *this * (1.0f / length());
    }

    inline operator float*()
    {
      return _data;
    }

    inline operator const float*() const
    {
      return _data;
    }

    inline bool is_inside_of (const vector_3d& a, const vector_3d& b ) const
    {
      return a.x < x && b.x > x
          && a.y < y && b.y > y
          && a.z < z && b.z > z;
    }
  };

  inline vector_3d min (vector_3d const& lhs, vector_3d const& rhs)
  {
    return { std::min (lhs.x, rhs.x)
           , std::min (lhs.y, rhs.y)
           , std::min (lhs.z, rhs.z)
           };
  }
  inline vector_3d max (vector_3d const& lhs, vector_3d const& rhs)
  {
    return { std::max (lhs.x, rhs.x)
           , std::max (lhs.y, rhs.y)
           , std::max (lhs.z, rhs.z)
           };
  }
}
