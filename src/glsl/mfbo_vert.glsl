// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 position;

uniform mat4 model_view_projection;

void main()
{
  gl_Position = model_view_projection * position;
}
