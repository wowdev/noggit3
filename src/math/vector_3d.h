// vector_3d.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace math
{
  class vector_3d
  {
  public:
    vector_3d (float x = 0.0f, float y = 0.0f, float z = 0.0f)
      : _x (x)
      , _y (y)
      , _z (z)
    {}

    vector_3d (const vector_3d& v)
      : _x (v._x)
      , _y (v._y)
      , _z (v._z)
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
      _x = v._x;
      _y = v._y;
      _z = v._z;
      return *this;
    }

    const float& x() const
    {
      return _x;
    }
    const float& y() const
    {
      return _y;
    }
    const float& z() const
    {
      return _z;
    }
    float& x()
    {
      return _x;
    }
    float& y()
    {
      return _y;
    }
    float& z()
    {
      return _z;
    }

    const float& x (const float& x_)
    {
      return _x = x_;
    }
    const float& y (const float& y_)
    {
      return _y = y_;
    }
    const float& z (const float& z_)
    {
      return _z = z_;
    }

    inline vector_3d operator+ (const vector_3d &v) const
    {
      return vector_3d (_x + v._x, _y + v._y, _z + v._z);
    }

    inline vector_3d operator- (const vector_3d &v) const
    {
      return vector_3d (_x - v._x, _y - v._y, _z - v._z);
    }

    inline float operator* (const vector_3d &v) const
    {
      return _x * v._x + _y * v._y + _z * v._z;
    }

    inline vector_3d operator* (const float& d) const
    {
      return vector_3d (_x * d, _y * d, _z * d);
    }

    friend vector_3d operator* (const float& d, const vector_3d& v)
    {
      return v * d;
    }

    inline vector_3d operator% (const vector_3d& v) const
    {
      return vector_3d ( _y * v._z - _z * v._y
                       , _z * v._x - _x * v._z
                       , _x * v._y - _y * v._x
                       );
    }

    inline vector_3d& operator+= (const vector_3d& v)
    {
      _x += v._x;
      _y += v._y;
      _z += v._z;
      return *this;
    }

    inline vector_3d& operator-= (const vector_3d& v)
    {
      _x -= v._x;
      _y -= v._y;
      _z -= v._z;
      return *this;
    }

    inline vector_3d& operator*= (float d)
    {
      _x *= d;
      _y *= d;
      _z *= d;
      return *this;
    }

    inline float length_squared() const
    {
      return _x * _x + _y * _y + _z * _z;
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
      return a._x < _x && b._x > _x
          && a._y < _y && b._y > _y
          && a._z < _z && b._z > _z;
    }

  private:
    union
    {
      float _data[3];
      struct
      {
        float _x;
        float _y;
        float _z;
      };
    };
  };

  inline vector_3d min (vector_3d const& lhs, vector_3d const& rhs)
  {
    return { std::min (lhs.x(), rhs.x())
           , std::min (lhs.y(), rhs.y())
           , std::min (lhs.z(), rhs.z())
           };
  }
  inline vector_3d max (vector_3d const& lhs, vector_3d const& rhs)
  {
    return { std::max (lhs.x(), rhs.x())
           , std::max (lhs.y(), rhs.y())
           , std::max (lhs.z(), rhs.z())
           };
  }
}
