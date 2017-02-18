// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>

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
    for (size_t side (0); side < SIDES_MAX; ++side)
    {
      if (_planes[side].normal() * point <= -_planes[side].distance())
      {
        return false;
      }
    }
    return true;
  }

  bool frustum::intersects ( const vector_3d& v1
                           , const vector_3d& v2
                           ) const
  {
    vector_3d points[8];
    points[0] = vector_3d (v1.x, v1.y, v1.z);
    points[1] = vector_3d (v1.x, v1.y, v2.z);
    points[2] = vector_3d (v1.x, v2.y, v1.z);
    points[3] = vector_3d (v1.x, v2.y, v2.z);
    points[4] = vector_3d (v2.x, v1.y, v1.z);
    points[5] = vector_3d (v2.x, v1.y, v2.z);
    points[6] = vector_3d (v2.x, v2.y, v1.z);
    points[7] = vector_3d (v2.x, v2.y, v2.z);


    for (size_t side (0); side < SIDES_MAX; ++side)
    {
      for (size_t point (0); point < 8; ++point)
      {
        if (_planes[side].normal() * points[point] > -_planes[side].distance())
        {
          //! \note C does not know how to continue out of two loops otherwise.
          goto intersects_next_side;
        }
      }

      return false;

    intersects_next_side: ;
    }

    return true;
  }


  bool frustum::intersectsSphere ( const vector_3d& position
                                 , const float& radius
                                 ) const
  {
    for (size_t side (0); side < SIDES_MAX; ++side)
    {
      const float distance ( _planes[side].normal() * position
                           + _planes[side].distance()
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
