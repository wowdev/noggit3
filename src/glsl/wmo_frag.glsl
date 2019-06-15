// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D tex1;

uniform bool use_vertex_color;

uniform bool draw_fog;
uniform bool unfogged;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
uniform vec3 camera;

in vec3 f_position;
in vec2 f_texcoord;
in vec4 f_vertex_color;

out vec4 out_color;

void main()
{
  float dist_from_camera = distance(camera, f_position);
  bool fog = draw_fog && !unfogged;
  vec4 color;

  if(fog && dist_from_camera >= fog_end)
  {
    out_color = vec4(fog_color, 1.);
    return;
  }

  vec4 tex = texture2D(tex1, f_texcoord);
  
  if(use_vertex_color) 
  {
    color = vec4(tex.rgb * f_vertex_color.rgb, tex.a);
  }
  else
  {
    color = tex;
  }

  if(fog && (dist_from_camera >= fog_end * fog_start))
  {
    float start = fog_end * fog_start;
    float alpha = (dist_from_camera - start) / (fog_end - start);

    color.rgb = mix(color.rgb, fog_color, alpha);
  }

  out_color = color;
}
