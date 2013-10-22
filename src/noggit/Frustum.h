// Frustum.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <math/vector_3d.h>

enum SIDES
{
  RIGHT,
  LEFT,
  BOTTOM,
  TOP,
  BACK,
  FRONT,
  SIDES_MAX,
};

class Frustum
{
  class plane
  {
  public:
    void normalize()
    {
      const float repri (_normal.length());
      _normal *= repri;
      _distance *= repri;
    }

    const float& distance() const
    {
      return _distance;
    }
    const float& distance (const float& distance_)
    {
      return _distance = distance_;
    }

    const ::math::vector_3d& normal() const
    {
      return _normal;
    }
    const ::math::vector_3d& normal (const ::math::vector_3d& normal_)
    {
      return _normal = normal_;
    }

  private:
    ::math::vector_3d _normal;
    float _distance;
  } _planes[SIDES_MAX];

public:
  Frustum();

  bool contains (const ::math::vector_3d& point) const;
  bool intersects ( const ::math::vector_3d& v1
                  , const ::math::vector_3d& v2
                  ) const;
  bool intersectsSphere ( const ::math::vector_3d& position
                        , const float& radius
                        ) const;
};


#endif

