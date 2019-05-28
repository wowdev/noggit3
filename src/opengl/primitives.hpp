// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

namespace math
{
  struct vector_3d;
  struct vector_4d;
}

namespace opengl
{
  namespace primitives
  {
    class wire_box
    {
    public:
      wire_box ( math::vector_3d const& min_point
               , math::vector_3d const& max_point
               );

      void draw ( math::matrix_4x4 const& model_view
                , math::matrix_4x4 const& projection
                , math::matrix_4x4 const& transform
                , math::vector_4d const& color
                , float line_width
                ) const;

      // for legacy stuff, todo: remove
      void draw( math::matrix_4x4 const& model_view
               , math::matrix_4x4 const& projection
               , math::vector_4d const& color
               , float line_width
               ) const;

    private:
      scoped::deferred_upload_vertex_arrays<1> _vao;
      scoped::buffers<2> _buffers;
      GLuint const& _positions = _buffers[0];
      GLuint const& _indices = _buffers[1];
      opengl::program _program;
    };
  }
}
