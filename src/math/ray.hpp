#pragma once

#include <boost/optional/optional.hpp>

#include <math/vector_3d.h>
#include <math/matrix_4x4.h>

namespace math
{
  struct ray
  {
    ray (vector_3d origin, vector_3d const& direction)
      : _origin (std::move (origin))
      , _direction (direction.normalized())
    {}

    ray (matrix_4x4 const& transform, ray const& other)
      : ray ( (transform * math::vector_4d (other._origin, 1.0)).xyz()
            , (transform * math::vector_4d (other._direction, 0.0)).xyz()
            )
    {}

    boost::optional<float> intersect_bounds (vector_3d const& _min, vector_3d const& _max) const;
    boost::optional<float> intersect_triangle (vector_3d const& _v0, vector_3d const& _v1, vector_3d const& _v2) const;

    vector_3d position (float distance) const
    {
      return _origin + _direction * distance;
    }

  private:
    vector_3d _origin;
    vector_3d _direction;
  };
}
