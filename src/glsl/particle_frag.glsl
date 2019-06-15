// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec2 f_uv;
in vec4 f_color;

out vec4 out_color;

uniform sampler2D tex;

uniform float alpha_test;

void main()
{
  vec4 t = texture(tex, f_uv);

  if(t.a < alpha_test)
  {
    discard;
  }

  out_color = vec4(f_color.rgb * t.rgb, t.a);
}
