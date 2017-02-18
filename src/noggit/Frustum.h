// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <math/matrix_4x4.hpp>

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
    plane() = default;
    plane (math::vector_4d const& vec)
      : _normal (vec.xyz())
      , _distance (vec.w)
    {
      normalize();
    }

    void normalize()
    {
      const float recip (1.0f / _normal.length());
      _normal *= recip;
      _distance *= recip;
    }

    const float& distance() const
    {
      return _distance;
    }

    const ::math::vector_3d& normal() const
    {
      return _normal;
    }

  private:
    ::math::vector_3d _normal;
    float _distance;
  } _planes[SIDES_MAX];

public:
  Frustum (::math::matrix_4x4 const& matrix);

  bool contains (const ::math::vector_3d& point) const;
  bool intersects ( const ::math::vector_3d& v1
                  , const ::math::vector_3d& v2
                  ) const;
  bool intersectsSphere ( const ::math::vector_3d& position
                        , const float& radius
                        ) const;
};
