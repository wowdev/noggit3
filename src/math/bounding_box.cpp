// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>

namespace
{
  math::vector_3d min_per_dimension(std::vector<math::vector_3d> const& points)
  {
    auto min(math::vector_3d::max());
    for (auto const& point : points)
    {
      min = math::min(min, point);
    }
    return min;
  }
  math::vector_3d max_per_dimension(std::vector<math::vector_3d> const& points)
  {
    auto max(math::vector_3d::min());
    for (auto const& point : points)
    {
      max = math::max(max, point);
    }
    return max;
  }
}

namespace math
{
  aabb::aabb(math::vector_3d const& min_, math::vector_3d const& max_)
    : min(min_)
    , max(max_)
  {
  }

  aabb::aabb(std::vector<math::vector_3d> points)
    : aabb(min_per_dimension(points), max_per_dimension(points))
  {
  }

  //! \todo Optimize: iterate lazily.
  std::vector<math::vector_3d> aabb::all_corners() const
  {
    return box_points(min, max);
  }


  std::vector<math::vector_3d> box_points(math::vector_3d const& box_min, math::vector_3d const& box_max)
  {
    std::vector<math::vector_3d> points;

    points.emplace_back(box_max.x, box_max.y, box_max.z);
    points.emplace_back(box_max.x, box_max.y, box_min.z);
    points.emplace_back(box_max.x, box_min.y, box_max.z);
    points.emplace_back(box_max.x, box_min.y, box_min.z);
    points.emplace_back(box_min.x, box_max.y, box_max.z);
    points.emplace_back(box_min.x, box_max.y, box_min.z);
    points.emplace_back(box_min.x, box_min.y, box_max.z);
    points.emplace_back(box_min.x, box_min.y, box_min.z);

    return points;
  }
}
