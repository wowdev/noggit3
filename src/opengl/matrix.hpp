// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>
#include <math/projection.hpp>
#include <math/trig.hpp>
#include <math/vector_3d.hpp>
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

    inline void perspective (math::degrees fovy, float aspect, float zNear, float zFar)
    {
      gl.multMatrixf (math::perspective (fovy, aspect, zNear, zFar).transposed());
    }

    inline void look_at ( ::math::vector_3d const& eye
                        , ::math::vector_3d const& center
                        , ::math::vector_3d const& up
                        )
    {
      gl.multMatrixf (math::look_at (eye, center, up).transposed());
    }
  }
}
