// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>

#include <vector>

namespace math
{
  frustum::frustum (matrix_4x4 const& matrix)
  {
    const vector_4d column_0 (matrix.column<0>());
    const vector_4d column_1 (matrix.column<1>());
    const vector_4d column_2 (matrix.column<2>());
    const vector_4d column_3 (matrix.column<3>());

    _planes[RIGHT] = column_3 - column_0;
    _planes[LEFT] = column_3 + column_0;
    _planes[TOP] = column_3 - column_1;
    _planes[BOTTOM] = column_3 + column_1;
    _planes[BACK] = column_3 - column_2;
    _planes[FRONT] = column_3 + column_2;
  }

  bool frustum::contains (const vector_3d& point) const
  {
    for (auto const& pl : _planes)
    {
      if (pl.normal() * point <= -pl.distance())
      {
        return false;
      }
    }
    return true;
  }

  bool frustum::intersects (const std::vector<vector_3d>& intersect_points) const
  {
    for (auto const& pl : _planes)
    {
      for (auto const& point : intersect_points)
      {
        if (pl.normal() * point > -pl.distance())
        {
          //! \note C does not know how to continue out of two loops otherwise.
          goto intersects_next_side;
        }
      }

      return false;

    intersects_next_side:;
    }

    return true;
  }

  bool frustum::intersects ( const vector_3d& v1
                           , const vector_3d& v2
                           ) const
  {
    std::vector<vector_3d> points;
    points.emplace_back (v1.x, v1.y, v1.z);
    points.emplace_back (v1.x, v1.y, v2.z);
    points.emplace_back (v1.x, v2.y, v1.z);
    points.emplace_back (v1.x, v2.y, v2.z);
    points.emplace_back (v2.x, v1.y, v1.z);
    points.emplace_back (v2.x, v1.y, v2.z);
    points.emplace_back (v2.x, v2.y, v1.z);
    points.emplace_back (v2.x, v2.y, v2.z);

    return intersects (points);
  }


  bool frustum::intersectsSphere ( const vector_3d& position
                                 , const float& radius
                                 ) const
  {
    for (auto const& pl : _planes)
    {
      const float distance ( pl.normal() * position
                           + pl.distance()
                           );
      if (distance < -radius)
      {
        return false;
      }
      else if (std::abs (distance) < radius)
      {
        return true;
      }
    }
    return true;
  }
}
