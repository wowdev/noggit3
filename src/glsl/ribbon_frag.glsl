// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec2 f_uv;

out vec4 out_color;

uniform sampler2D tex;
uniform vec4 color;

void main()
{
  vec4 t = texture(tex, f_uv);
  out_color = vec4(color.rgb * t.rgb, color.a);
}
