// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in mat4 transform;
in vec4 position;
in vec3 offset;
in vec2 uv;
in vec4 color;

out vec2 f_uv;
out vec4 f_color;

uniform mat4 model_view_projection;
uniform int billboard;

void main()
{
  f_uv = uv;
  f_color = color;
  if(billboard == 1)
  { 
    vec4 pos = transform*position;
    pos.xyz += offset;
    gl_Position = model_view_projection * pos;
  }
  else
  {
    gl_Position = model_view_projection * transform * position;
  }
}
