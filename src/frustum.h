#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "Vec3D.h"

struct Plane {
  float a,b,c,d;
  void normalize();
};

enum Directions {
  RIGHT, LEFT, BOTTOM, TOP, BACK, FRONT
};

struct Frustum {
  Plane planes[6];

  void retrieve();

  bool contains(const Vec3D &v) const;
  bool intersects(const Vec3D &v1, const Vec3D &v2) const;
  bool intersectsSphere(const Vec3D& v, const float rad) const;
};


#endif

