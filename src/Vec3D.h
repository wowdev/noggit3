#ifndef VEC3D_H
#define VEC3D_H

#include <cmath>

class Vec3D
{
public:
  float x;
  float y;
  float z;

  Vec3D(float x0 = 0.0f, float y0 = 0.0f, float z0 = 0.0f)
  : x(x0)
  , y(y0)
  , z(z0)
  {}

  Vec3D(const Vec3D& v)
  : x(v.x)
  , y(v.y)
  , z(v.z)
  {}

  inline Vec3D& operator= (const Vec3D &v)
  {
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
  }

  inline Vec3D operator+ (const Vec3D &v) const
  {
    return Vec3D(x + v.x, y + v.y, z + v.z);
  }

  inline Vec3D operator- (const Vec3D &v) const
  {
    return Vec3D(x - v.x, y - v.y, z - v.z);
  }

  inline float operator* (const Vec3D &v) const
  {
    return x * v.x + y * v.y + z * v.z;
  }

  inline Vec3D operator* (const float& d) const
  {
    return Vec3D(x * d, y * d, z * d);
  }

  friend Vec3D operator* (const float& d, const Vec3D& v)
  {
    return v * d;
  }

  inline Vec3D operator% (const Vec3D& v) const
  {
    return Vec3D(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }

  inline Vec3D& operator+= (const Vec3D& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  inline Vec3D& operator-= (const Vec3D& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }

  inline Vec3D& operator*= (float d)
  {
    x *= d;
    y *= d;
    z *= d;
    return *this;
  }

  inline float lengthSquared() const
  {
    return x * x + y * y + z * z;
  }

  inline float length() const
  {
    return sqrt(lengthSquared());
  }

  inline Vec3D& normalize()
  {
    const float repri (1.0f / length());
    x *= repri;
    y *= repri;
    z *= repri;
    return *this;
  }

  inline Vec3D operator~ () const
  {
    Vec3D r(*this);
    r.normalize();
    return r;
  }

  inline operator float*()
  {
    return reinterpret_cast<float*>(this);
  }

  inline operator const float*() const
  {
    return reinterpret_cast<const float*>(this);
  }

  inline bool IsInsideOf( Vec3D pA, Vec3D pB ) const
  {
    return( pA.x < this->x && pB.x > this->x && pA.y < this->y && pB.y > this->y && pA.z < this->z && pB.z > this->z );
  }
};


class Vec2D {
public:
  float x;
  float y;

  Vec2D(float x0 = 0.0f, float y0 = 0.0f)
  : x(x0)
  , y(y0)
  {}

  Vec2D(const Vec2D& v)
  : x(v.x)
  , y(v.y)
  {}

  inline Vec2D& operator= (const Vec2D &v)
  {
    x = v.x;
    y = v.y;
    return *this;
  }

  inline Vec2D operator+ (const Vec2D &v) const
  {
    return Vec2D(x + v.x, y + v.y);
  }

  inline Vec2D operator- (const Vec2D &v) const
  {
    return Vec2D(x - v.x, y - v.y);
  }

  inline float operator* (const Vec2D &v) const
  {
    return x*v.x + y*v.y;
  }

  inline Vec2D operator* (float d) const
  {
    return Vec2D(x * d, y * d);
  }

  friend Vec2D operator* (float d, const Vec2D& v)
  {
    return v * d;
  }

  inline Vec2D& operator+= (const Vec2D &v)
  {
    x += v.x;
    y += v.y;
    return *this;
  }

  inline Vec2D& operator-= (const Vec2D &v)
  {
    x -= v.x;
    y -= v.y;
    return *this;
  }

  inline Vec2D& operator*= (float d)
  {
    x *= d;
    y *= d;
    return *this;
  }

  inline float lengthSquared() const
  {
    return x * x + y * y;
  }

  inline float length() const
  {
    return sqrt(lengthSquared());
  }

  inline Vec2D& normalize()
  {
    const float repri (1.0f / length());
    x *= repri;
    y *= repri;
    return *this;
  }

  inline Vec2D operator~ () const
  {
    Vec2D r(*this);
    r.normalize();
    return r;
  }

  inline operator float*()
  {
    return reinterpret_cast<float*>(this);
  }

};


inline void rotate(float x0, float y0, float *x, float *y, float angle)
{
  float xa = *x - x0, ya = *y - y0;
  *x = xa * cosf(angle) - ya * sinf(angle) + x0;
  *y = xa * sinf(angle) + ya * cosf(angle) + y0;
}



#endif

