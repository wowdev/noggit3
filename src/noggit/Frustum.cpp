// Frustum.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/Frustum.h>

#include <opengl/types.h>

#include <math/matrix_4x4.h>

Frustum::Frustum()
{
  ::math::matrix_4x4 matrix;

  glGetFloatv (GL_MODELVIEW_MATRIX, matrix);
  glMatrixMode (GL_PROJECTION);

  glPushMatrix();

  glMultMatrixf (matrix);
  glGetFloatv (GL_PROJECTION_MATRIX, matrix);

  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);

  const ::math::vector_4d column_0 (matrix.column<0>());
  const ::math::vector_4d column_1 (matrix.column<1>());
  const ::math::vector_4d column_2 (matrix.column<2>());
  const ::math::vector_4d column_3 (matrix.column<3>());

  {
    const ::math::vector_4d temp (column_3 - column_0);
    planes[RIGHT].normal (::math::vector_3d(temp.x(),temp.y(),temp.z()) );
    planes[RIGHT].distance (temp.w());
    planes[RIGHT].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 + column_0);
    planes[LEFT].normal (::math::vector_3d(temp.x(),temp.y(),temp.z()));
    planes[LEFT].distance (temp.w());
    planes[LEFT].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 - column_1);
    planes[BOTTOM].normal (::math::vector_3d(temp.x(),temp.y(),temp.z()));
    planes[BOTTOM].distance (temp.w());
    planes[BOTTOM].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 + column_1);
    planes[TOP].normal (::math::vector_3d(temp.x(),temp.y(),temp.z()));
    planes[TOP].distance (temp.w());
    planes[TOP].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 - column_2);
    planes[BACK].normal (::math::vector_3d(temp.x(),temp.y(),temp.z()));
    planes[BACK].distance (temp.w());
    planes[BACK].normalize();
  }
  {
    const ::math::vector_4d temp (column_3 + column_2);
    planes[FRONT].normal (::math::vector_3d(temp.x(),temp.y(),temp.z()));
    planes[FRONT].distance (temp.w());
    planes[FRONT].normalize();
  }
}

bool Frustum::contains (const ::math::vector_3d& point) const
{
  for (size_t side (0); side < SIDES_MAX; ++side)
  {
    if (planes[side].normal() * point <= -planes[side].distance())
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
      if (planes[side].normal() * points[point] >= -planes[side].distance())
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
    const float distance ( planes[side].normal() * position
                         + planes[side].distance()
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

