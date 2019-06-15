// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D tex1;

in vec2 f_texcoord;

out vec4 out_color;

void main()
{
  out_color = texture2D(tex1, f_texcoord);
}
