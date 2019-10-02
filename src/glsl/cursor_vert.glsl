// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform mat4 model_view_projection;
uniform vec3 cursor_pos;
uniform float radius;

in vec4 position;

void main()
{
  vec3 p = cursor_pos + position.xyz * radius;
  gl_Position = model_view_projection * vec4(p,1.);
}
