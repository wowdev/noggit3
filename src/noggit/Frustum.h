#ifndef FRUSTUM_H
#define FRUSTUM_H

namespace math
{
  class vector_3d;
}

struct Plane
{
  float a,b,c,d;
  void normalize();
};

enum Directions
{
  RIGHT, LEFT, BOTTOM, TOP, BACK, FRONT
};

struct Frustum
{
  Plane planes[6];

  void retrieve();

  bool contains (const ::math::vector_3d& v) const;
  bool intersects (const ::math::vector_3d& v1, const ::math::vector_3d& v2) const;
  bool intersectsSphere (const ::math::vector_3d& v, const float rad) const;
};


#endif

