// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 position;
in vec3 normal;
in vec3 color;
in vec2 texcoord;

out vec2 f_texcoord;


uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

void main()
{
  gl_Position = projection * model_view * transform * position;

  f_texcoord = texcoord;
}
