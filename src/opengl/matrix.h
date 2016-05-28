// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/constants.h>
#include <math/matrix_4x4.h>
#include <math/vector_3d.h>

#include <opengl/context.h>
#include <opengl/types.h>

#include <cmath>

namespace opengl
{
  namespace matrix
  {
    template<GLint matrix_type>
      inline ::math::matrix_4x4 from()
    {
      ::math::matrix_4x4 matrix;
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

    inline void perspective (float fovy, float aspect, float zNear, float zFar)
    {
      float const ymax (zNear * std::tan (fovy * ::math::constants::pi() / 360.0));
      gl.frustum (-ymax * aspect, ymax * aspect, -ymax, ymax, zNear, zFar);
    }

    inline void look_at ( ::math::vector_3d const& eye
                        , ::math::vector_3d const& center
                        , ::math::vector_3d const& up
                        )
    {
      ::math::vector_3d const z ((eye - center).normalized());
      ::math::vector_3d const x ((up % z).normalized());
      ::math::vector_3d const y ((z % x).normalized());
      ::math::matrix_4x4 const matrix ( x.x(),    y.x(),    z.x(),     0.0f
                                      , x.y(),    y.y(),    z.y(),     0.0f
                                      , x.z(),    y.z(),    z.z(),     0.0f
                                      , x * -eye, y * -eye, z * -eye,  1.0f
                                      );

      gl.multMatrixf (matrix);
    }
  }
}
