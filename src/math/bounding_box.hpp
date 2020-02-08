// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>

#include <vector>

namespace math
{
  struct aabb
  {
    aabb(math::vector_3d const& min_, math::vector_3d const& max_);
    aabb(std::vector<math::vector_3d> points);

    std::vector<math::vector_3d> all_corners() const;

    math::vector_3d min;
    math::vector_3d max;
  };

  std::vector<math::vector_3d> box_points(math::vector_3d const& box_min, math::vector_3d const& box_max);
}
