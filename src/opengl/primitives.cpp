// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/vector_4d.hpp>
#include <opengl/context.hpp>
#include <opengl/matrix.hpp>
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
#version 110

attribute vec4 position;

uniform mat4 model_view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * model_view * position;
}
)code"
                   }
                 , { GL_FRAGMENT_SHADER
                   , R"code(
#version 110

uniform vec4 color;

void main()
{
  gl_FragColor = color;
}
)code"
                   }
                 }
    {
      std::vector<math::vector_3d> positions;
      positions.reserve (8);
      positions.emplace_back (max_point.x, max_point.y, max_point.z);
      positions.emplace_back (max_point.x, max_point.y, min_point.z);
      positions.emplace_back (max_point.x, min_point.y, max_point.z);
      positions.emplace_back (max_point.x, min_point.y, min_point.z);
      positions.emplace_back (min_point.x, max_point.y, max_point.z);
      positions.emplace_back (min_point.x, max_point.y, min_point.z);
      positions.emplace_back (min_point.x, min_point.y, max_point.z);
      positions.emplace_back (min_point.x, min_point.y, min_point.z);

      static std::array<unsigned char, 16> const indices
        {{5, 7, 3, 2, 0, 1, 3, 1, 5, 4, 0, 4, 6, 2, 6, 7}};

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

    void wire_box::draw (math::vector_4d const& color, float line_width) const
    {
      opengl::scoped::use_program wire_box_shader {_program};

      opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);
      gl.lineWidth (line_width);

      wire_box_shader.uniform ("model_view", opengl::matrix::model_view());
      wire_box_shader.uniform ("projection", opengl::matrix::projection());

      wire_box_shader.attrib ("position", _positions, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      wire_box_shader.uniform ("color", color);

      gl.drawElements (GL_LINE_STRIP, _indices, 16, GL_UNSIGNED_BYTE, 0);
    }
  }
}
