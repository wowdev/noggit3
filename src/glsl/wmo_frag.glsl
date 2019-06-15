// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D tex1;

uniform bool use_vertex_color;

in vec2 f_texcoord;
in vec4 f_vertex_color;

out vec4 out_color;

void main()
{
  vec4 tex = texture2D(tex1, f_texcoord);
  
  if(use_vertex_color) 
  {
    out_color = vec4(tex.rgb * f_vertex_color.rgb, tex.a);
  }
  else
  {
    out_color = tex;
  }
}
