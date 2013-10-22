// matrix.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __OPENGL_MATRIX_H
#define __OPENGL_MATRIX_H

#include <math/matrix_4x4.h>

#include <opengl/types.h>

namespace opengl
{
  namespace matrix
  {
    template<GLint matrix_type>
    ::math::matrix_4x4 from()
    {
      ::math::matrix_4x4 matrix;
      glGetFloatv (matrix_type, matrix);
      return matrix;
    }

    ::math::matrix_4x4 model_view()
    {
      return from<GL_MODELVIEW_MATRIX>();
    }
    ::math::matrix_4x4 projection()
    {
      return from<GL_PROJECTION_MATRIX>();
    }
  }
}

#endif
