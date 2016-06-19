#pragma once

#include <boost/optional/optional.hpp>

#include <math/vector_3d.h>
#include <math/matrix_4x4.h>

namespace math
{
  struct ray
  {
    vector_3d origin;
    vector_3d direction;

    ray (vector_3d origin_, vector_3d const& direction_)
      : origin (std::move (origin_))
      , direction (direction_.normalized())
    {}

    ray (matrix_4x4 const& transform, ray const& other)
      : ray ( (transform * math::vector_4d (other.origin, 1.0)).xyz()
            , (transform * math::vector_4d (other.direction, 0.0)).xyz()
            )
    {}

    vector_3d position (float distance) const
    {
      return origin + direction * distance;
    }
  };
  boost::optional<float> intersect_bounds(ray const& _ray, vector_3d const& _min, vector_3d const& _max);
  boost::optional<float> intersect_triangle(ray const& _ray, vector_3d const& _v0, vector_3d const& _v1, vector_3d const& _v2);
}
