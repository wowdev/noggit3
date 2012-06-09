// Frustum.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/Frustum.h>

#include <opengl/matrix.h>
#include <opengl/types.h>

#include <math/matrix_4x4.h>

Frustum::Frustum()
{
  const ::math::matrix_4x4 matrix
    (::opengl::matrix::model_view() * ::opengl::matrix::projection());


  const ::math::vector_4d column_0 (matrix.column<0>());
  const ::math::vector_4d column_1 (matrix.column<1>());
  const ::math::vector_4d column_2 (matrix.column<2>());
  const ::math::vector_4d column_3 (matrix.column<3>());

  {
    const ::math::vector_4d temp (column_3 - column_0);
    _planes[RIGHT].normal (temp.xyz());
    _planes[RIGHT].distance (temp.w());
    _planes[RIGHT].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 + column_0);
    _planes[LEFT].normal (temp.xyz());
    _planes[LEFT].distance (temp.w());
    _planes[LEFT].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 - column_1);
    _planes[TOP].normal (temp.xyz());
    _planes[TOP].distance (temp.w());
    _planes[TOP].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 + column_1);
    _planes[BOTTOM].normal (temp.xyz());
    _planes[BOTTOM].distance (temp.w());
    _planes[BOTTOM].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 - column_2);
    _planes[BACK].normal (temp.xyz());
    _planes[BACK].distance (temp.w());
    _planes[BACK].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 + column_2);
    _planes[FRONT].normal (temp.xyz());
    _planes[FRONT].distance (temp.w());
    _planes[FRONT].normalize();
  }
}

bool Frustum::contains (const ::math::vector_3d& point) const
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

bool Frustum::intersects ( const ::math::vector_3d& v1
                         , const ::math::vector_3d& v2
                         ) const
{
  ::math::vector_3d points[8];
  points[0] = ::math::vector_3d (v1.x(), v1.y(), v1.z());
  points[1] = ::math::vector_3d (v1.x(), v1.y(), v2.z());
  points[2] = ::math::vector_3d (v1.x(), v2.y(), v1.z());
  points[3] = ::math::vector_3d (v1.x(), v2.y(), v2.z());
  points[4] = ::math::vector_3d (v2.x(), v1.y(), v1.z());
  points[5] = ::math::vector_3d (v2.x(), v1.y(), v2.z());
  points[6] = ::math::vector_3d (v2.x(), v2.y(), v1.z());
  points[7] = ::math::vector_3d (v2.x(), v2.y(), v2.z());


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


bool Frustum::intersectsSphere ( const ::math::vector_3d& position
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
    else if (fabsf (distance) < radius)
    {
      return true;
    }
  }
  return true;
}

