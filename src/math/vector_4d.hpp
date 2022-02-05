// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>

namespace math
{
  struct vector_4d
  {
    union
    {
      float _data[4];
      struct
      {
        float x;
        float y;
        float z;
        float w;
      };
    };

    vector_4d() : vector_4d (0.f, 0.f, 0.f, 0.f) {}
    vector_4d (float x_, float y_, float z_, float w_)
      : x (x_)
      , y (y_)
      , z (z_)
      , w (w_)
    { }

    vector_4d (const vector_4d& v)
      : x (v.x)
      , y (v.y)
      , z (v.z)
      , w (v.w)
    { }

    vector_4d (const ::math::vector_3d& v, const float w_)
      : x (v.x)
      , y (v.y)
      , z (v.z)
      , w (w_)
    { }

    vector_4d& operator= (const vector_4d &v)
    {
      x = v.x;
      y = v.y;
      z = v.z;
      w = v.w;
      return *this;
    }

    vector_3d xyz() const
    {
      return vector_3d (x, y, z);
    }
    const vector_3d& xyz (const vector_3d& xyz_)
    {
      x = xyz_.x;
      y = xyz_.y;
      z = xyz_.z;
      return xyz_;
    }
    vector_3d xyz_normalized_by_w() const
    {
      return vector_3d (x / w, y / w, z / w);
    }

    vector_4d operator+ (const vector_4d &v) const
    {
      return vector_4d (x + v.x, y + v.y, z + v.z, w + v.w);
    }

    vector_4d operator- (const vector_4d &v) const
    {
      return vector_4d (x - v.x, y - v.y, z - v.z, w - v.w);
    }

    vector_4d operator* (float d) const
    {
      return vector_4d (x * d, y * d, z * d, w * d);
    }

    float operator* (const vector_4d& v) const
    {
      return x * v.x + y * v.y + z * v.z + w * v.w;
    }

    vector_4d& operator*= (float d)
    {
      x *= d;
      y *= d;
      z *= d;
      w *= d;
      return *this;
    }

    vector_4d& normalize()
    {
      return operator *= (1.0f / std::sqrt (x * x + y * y + z * z + w * w));
    }

    operator const float*() const
    {
      return _data;
    }
    operator float*()
    {
      return _data;
    }
  };
}
