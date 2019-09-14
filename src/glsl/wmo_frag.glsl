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

uniform bool exterior_lit;
uniform vec3 exterior_light_dir;
uniform vec3 exterior_diffuse_color;
uniform vec3 exterior_ambient_color;

uniform float alpha_test;

in vec3 f_position;
in vec3 f_normal;
in vec2 f_texcoord;
in vec4 f_vertex_color;

out vec4 out_color;

void main()
{
  float dist_from_camera = distance(camera, f_position);
  bool fog = draw_fog && !unfogged;

  if(fog && dist_from_camera >= fog_end)
  {
    out_color = vec4(fog_color, 1.);
    return;
  }

  vec4 tex = texture2D(tex1, f_texcoord);

  if(tex.a < alpha_test)
  {
    discard;
  }
  
  if(use_vertex_color) 
  {
    out_color = vec4(tex.rgb * f_vertex_color.rgb, tex.a);
  }
  else
  {
    out_color = tex;
  }

  if(exterior_lit)
  {
    out_color.rgb *= vec3(clamp (exterior_diffuse_color * max(dot(f_normal, exterior_light_dir), 0.0), 0.0, 1.0)) + exterior_ambient_color;
  }

  if(fog && (dist_from_camera >= fog_end * fog_start))
  {
    float start = fog_end * fog_start;
    float alpha = (dist_from_camera - start) / (fog_end - start);

    out_color.rgb = mix(out_color.rgb, fog_color, alpha);
  }

  if(out_color.a < alpha_test)
  {
    discard;
  }
}
