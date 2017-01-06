// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>

#include <opengl/context.hpp>

namespace opengl
{
  namespace matrix
  {
    template<GLint matrix_type>
      inline ::math::matrix_4x4 from()
    {
      ::math::matrix_4x4 matrix (math::matrix_4x4::uninitialized);
      gl.getFloatv (matrix_type, matrix);
      return matrix;
    }

    inline ::math::matrix_4x4 model_view()
    {
      return from<GL_MODELVIEW_MATRIX>();
    }
    inline ::math::matrix_4x4 projection()
    {
      return from<GL_PROJECTION_MATRIX>();
    }
  }
}
