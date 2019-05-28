// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/matrix_4x4.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Misc.h>
#include <opengl/context.hpp>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>
#include <opengl/types.hpp>

#include <array>

namespace opengl
{
  namespace primitives
  {
    wire_box::wire_box ( math::vector_3d const& min_point
                       , math::vector_3d const& max_point
                       )
      : _program { { GL_VERTEX_SHADER
                   , R"code(
#version 330 core

in vec4 position;

uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

void main()
{
  gl_Position = projection * model_view * transform * position;
}
)code"
                   }
                 , { GL_FRAGMENT_SHADER
                   , R"code(
#version 330 core

uniform vec4 color;

out vec4 out_color;

void main()
{
  out_color = color;
}
)code"
                   }
                 }
    {
      std::vector<math::vector_3d> positions (misc::box_points (min_point, max_point));
      
      static std::array<std::uint8_t, 16> const indices
        {{5, 7, 3, 2, 0, 1, 3, 1, 5, 4, 0, 4, 6, 2, 6, 7}};

      _vao.upload();

      {
        scoped::buffer_binder<GL_ARRAY_BUFFER> const buffer (_positions);
        gl.bufferData ( GL_ARRAY_BUFFER
                      , positions.size() * sizeof (*positions.data())
                      , positions.data()
                      , GL_STATIC_DRAW
                      );
      }

      {
        scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const buffer (_indices);
        gl.bufferData ( GL_ELEMENT_ARRAY_BUFFER
                      , indices.size() * sizeof (*indices.data())
                      , indices.data()
                      , GL_STATIC_DRAW
                      );
      }
    }

    void wire_box::draw ( math::matrix_4x4 const& model_view
                        , math::matrix_4x4 const& projection
                        , math::matrix_4x4 const& transform
                        , math::vector_4d const& color
                        , float line_width
                        ) const
    {
      opengl::scoped::use_program wire_box_shader {_program};

      wire_box_shader.uniform("model_view", model_view);
      wire_box_shader.uniform("projection", projection);
      wire_box_shader.uniform("transform", transform);
      wire_box_shader.uniform("color", color);

      opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      gl.lineWidth(line_width);
      
      opengl::scoped::vao_binder const _(_vao[0]);
      scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices (_positions);
      wire_box_shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

      scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const indices (_indices);

      gl.drawElements (GL_LINE_STRIP, _indices, 16, GL_UNSIGNED_SHORT, nullptr);
    }

    void wire_box::draw( math::matrix_4x4 const& model_view
                       , math::matrix_4x4 const& projection
                       , math::vector_4d const& color
                       , float line_width
                       ) const
    {
      draw(model_view, projection, math::matrix_4x4::unit, color, line_width);
    }
  }
}
