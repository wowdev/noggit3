// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <ostream>
#include <tuple>
#include <vector>

namespace math
{
  template<typename T>
    struct vector_3d_base
  {
    union
    {
      T _data[3];
      struct
      {
        T x;
        T y;
        T z;
      };
    };

    vector_3d_base<T>() : vector_3d_base (T(0), T(0), T(0)) {}
    vector_3d_base<T> (T x_, T y_, T z_)
      : x (x_)
      , y (y_)
      , z (z_)
    {}

    template<typename U>
        explicit vector_3d_base<T> (vector_3d_base<U> const& other)
      : x (other.x)
      , y (other.y)
      , z (other.z)
    {}

    inline static vector_3d_base<T> min()
    {
      return {std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()};
    }
    inline static vector_3d_base<T> max()
    {
      return {std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max()};
    }

    vector_3d_base<T> (vector_3d_base<T> const&) = default;
    vector_3d_base<T> (vector_3d_base<T>&&) = default;
    vector_3d_base<T>& operator= (vector_3d_base<T> const&) = default;
    vector_3d_base<T>& operator= (vector_3d_base<T>&&) = default;

    inline vector_3d_base<T> operator+ (const vector_3d_base<T> &v) const
    {
      return vector_3d_base<T> (x + v.x, y + v.y, z + v.z);
    }

    inline vector_3d_base<T> operator- (const vector_3d_base<T> &v) const
    {
      return vector_3d_base<T> (x - v.x, y - v.y, z - v.z);
    }

    inline vector_3d_base<T> operator-() const
    {
      return vector_3d_base<T> (-x, -y, -z);
    }

    inline T operator* (const vector_3d_base<T> &v) const
    {
      return x * v.x + y * v.y + z * v.z;
    }

    inline T operator/ (const vector_3d_base<T>& v) const
    {
      return x / v.x + y / v.y + z / v.z;
    }

    inline vector_3d_base<T> operator* (const T& d) const
    {
      return vector_3d_base<T> (x * d, y * d, z * d);
    }

    inline vector_3d_base<T> operator/ (const T& d) const
    {
      return vector_3d_base<T>(x / d, y / d, z / d);
    }

    friend vector_3d_base<T> operator* (const T& d, const vector_3d_base<T>& v)
    {
      return v * d;
    }

    friend vector_3d_base<T> operator/ (const T& d, const vector_3d_base<T>& v)
    {
      return v / d;
    }

    inline vector_3d_base<T> operator% (const vector_3d_base<T>& v) const
    {
      return vector_3d_base<T> ( y * v.z - z * v.y
                       , z * v.x - x * v.z
                       , x * v.y - y * v.x
                       );
    }

    inline vector_3d_base<T>& operator+= (const vector_3d_base<T>& v)
    {
      x += v.x;
      y += v.y;
      z += v.z;
      return *this;
    }

    inline vector_3d_base<T>& operator-= (const vector_3d_base<T>& v)
    {
      x -= v.x;
      y -= v.y;
      z -= v.z;
      return *this;
    }

    inline vector_3d_base<T>& operator*= (T d)
    {
      x *= d;
      y *= d;
      z *= d;
      return *this;
    }

    inline vector_3d_base<T>& operator/= (T d)
    {
      x /= d;
      y /= d;
      z /= d;
      return *this;
    }

    inline T length_squared() const
    {
      return x * x + y * y + z * z;
    }

    inline T length() const
    {
      return std::sqrt (length_squared());
    }

    inline vector_3d_base<T>& normalize()
    {
      return operator *= (1.0f / length());
    }

    vector_3d_base<T> normalized() const
    {
      return *this * (1.0f / length());
    }

    inline operator T*()
    {
      return _data;
    }

    inline operator const T*() const
    {
      return _data;
    }

    inline bool is_inside_of (const vector_3d_base<T>& a, const vector_3d_base<T>& b ) const
    {
      return a.x < x && b.x > x
          && a.y < y && b.y > y
          && a.z < z && b.z > z;
    }

    bool operator== (vector_3d_base<T> const& rhs) const
    {
      return std::tie (x, y, z) == std::tie (rhs.x, rhs.y, rhs.z);
    }
    friend std::ostream& operator<< (std::ostream& os, vector_3d_base<T> const& x)
    {
      return os << x.x << ", " << x.y << ", " << x.z;
    }

  };

  template<typename T>
    inline vector_3d_base<T> min (vector_3d_base<T> const& lhs, vector_3d_base<T> const& rhs)
  {
    return { std::min (lhs.x, rhs.x)
           , std::min (lhs.y, rhs.y)
           , std::min (lhs.z, rhs.z)
           };
  }
  template<typename T>
    inline vector_3d_base<T> max (vector_3d_base<T> const& lhs, vector_3d_base<T> const& rhs)
  {
    return { std::max (lhs.x, rhs.x)
           , std::max (lhs.y, rhs.y)
           , std::max (lhs.z, rhs.z)
           };
  }

  struct vector_3d : public vector_3d_base<float>
  {
  public:
    using vector_3d_base<float>::vector_3d_base;
    vector_3d (vector_3d_base<float> v) : vector_3d_base<float> (std::move (v)) {}
    vector_3d() : vector_3d_base<float>() {}
  };


  template<typename Fun>
    std::vector<math::vector_3d> apply
      (Fun&& fun, std::vector<math::vector_3d> points)
  {
    for (auto& point : points)
    {
      point = fun (point);
    }
    return points;
  }
}
