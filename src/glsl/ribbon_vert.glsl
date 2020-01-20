// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in mat4 transform;
in vec4 position;
in vec2 uv;

out vec2 f_uv;

uniform mat4 model_view_projection;

void main()
{
  f_uv = uv;
  gl_Position = model_view_projection * transform * position;
}
