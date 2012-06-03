// vector_4d.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef QUATERNION_H
#define QUATERNION_H

#include <math/vector_3d.h>

namespace math
{
  class vector_4d
  {
  public:
    vector_4d ( const float& x = 0.0f
              , const float& y = 0.0f
              , const float& z = 0.0f
              , const float& w = 0.0f
              )
      : _x (x)
      , _y (y)
      , _z (z)
      , _w (w)
    { }

    vector_4d (const vector_4d& v)
      : _x (v._x)
      , _y (v._y)
      , _z (v._z)
      , _w (v._w)
    { }

    vector_4d (const ::math::vector_3d& v, const float w)
      : _x (v.x())
      , _y (v.y())
      , _z (v.z())
      , _w (w)
    { }

    vector_4d& operator= (const vector_4d &v)
    {
      _x = v._x;
      _y = v._y;
      _z = v._z;
      _w = v._w;
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
    const float& w() const
    {
      return _w;
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
    const float& w (const float& w_)
    {
      return _w = w_;
    }

    const vector_3d& xyz() const
    {
      // this is only temporary so doesent work
      return vector_3d (_x, _y, _z);
    }
    const vector_3d& xyz (const vector_3d& xyz_)
    {
      _x = xyz_.x();
      _y = xyz_.y();
      _z = xyz_.z();
      return xyz_;
    }

    vector_4d operator+ (const vector_4d &v) const
    {
      return vector_4d (_x + v._x, _y + v._y, _z + v._z, _w + v._w);
    }

    vector_4d operator- (const vector_4d &v) const
    {
      return vector_4d (_x - v._x, _y - v._y, _z - v._z, _w - v._w);
    }

    vector_4d operator* (float d) const
    {
      return vector_4d (_x * d, _y * d, _z * d, _w * d);
    }

    float operator* (const vector_4d& v) const
    {
      return _x * v._x + _y * v._y + _z * v._z + _w * v._w;
    }

    vector_4d& operator*= (float d)
    {
      _x *= d;
      _y *= d;
      _z *= d;
      _w *= d;
      return *this;
    }

    vector_4d& normalize()
    {
      return operator *= (1.0f / sqrtf (_x * _x + _y * _y + _z * _z + _w * _w));
    }

    operator const float*() const
    {
      return _data;
    }
    operator float*()
    {
      return _data;
    }

  private:
    union
    {
      float _data[4];
      struct
      {
        float _x;
        float _y;
        float _z;
        float _w;
      };
    };

    friend class Matrix;
  };
}

#endif
