// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/cursor_render.hpp>

#include <opengl/shader.hpp>

namespace noggit
{
  void cursor_render::draw(mode cursor_mode, math::matrix_4x4 const& mvp, math::vector_4d color, math::vector_3d pos, float radius, float inner_radius_ratio)
  {
    if (!_uploaded)
    {
      upload();
    }

    static std::uint16_t indice = 0;

    gl.bufferData<GL_ARRAY_BUFFER>(_vertex_vbo, sizeof(math::vector_3d), &pos, GL_STATIC_DRAW);

    opengl::scoped::use_program shader {*_cursor_program.get()};

    shader.uniform("model_view_projection", mvp);
    shader.uniform("color", color);
    shader.uniform("radius", radius);
    shader.uniform("inner_radius_ratio", inner_radius_ratio);

    opengl::scoped::vao_binder const _ (_vao);
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const buffer (_vertex_vbo);
    shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    gl.drawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, &indice);
  }

  void cursor_render::upload()
  {
    _vaos.upload();
    _vbos.upload();

    _cursor_program.reset(new opengl::program(
      {
          {GL_VERTEX_SHADER, R"code(
#version 330 core

in vec4 position;

void main()
{
  gl_Position = position;
}
)code" }
        , {GL_GEOMETRY_SHADER, R"code(
#version 330 core

layout(points) in;
layout(line_strip, max_vertices = 256) out;

float PI = 3.14159265;

uniform mat4 model_view_projection;
uniform float radius;
uniform float inner_radius_ratio;

void line(vec4 p1, vec4 p2)
{
  gl_Position = model_view_projection * p1;
  EmitVertex();
  gl_Position = model_view_projection * p2;
  EmitVertex();
  EndPrimitive();
}

void circle(vec4 origin, float r, int segment)
{
  float div = float(segment);

  for(float i=0; i<segment; ++i)
  {
      vec4 p1 = origin;
      p1.x += r * cos(2*PI*float(i)/div);
      p1.z += r * sin(2*PI*float(i)/div);

    

    vec4 p2 = origin;
    p2.x += r * cos(2*PI*float(i+1)/div);
    p2.z += r * sin(2*PI*float(i+1)/div);

    line(p1, p2);
  }
}

void main()
{
  vec4 origin = gl_in[0].gl_Position;

  circle(origin, radius, 48);
  if(inner_radius_ratio > 0. && inner_radius_ratio < 1.)
  {
    circle(origin, radius*inner_radius_ratio, 48);
  }  
}
)code" }
        , {GL_FRAGMENT_SHADER, R"code(
#version 330 core

uniform vec4 color;

out vec4 out_color;

void main()
{
  out_color = color;
}
)code" }
      }
    ));

    _uploaded = true;
  }
}
