// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform bool use_vertex_color;

uniform bool draw_fog;
uniform bool unfogged;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
uniform vec3 camera;

uniform bool unlit;
uniform bool exterior_lit;
uniform vec3 exterior_light_dir;
uniform vec3 exterior_diffuse_color;
uniform vec3 exterior_ambient_color;
uniform vec3 ambient_color;

uniform float alpha_test;

uniform int shader_id;

in vec3 f_position;
in vec3 f_normal;
in vec2 f_texcoord;
in vec2 f_texcoord_2;
in vec4 f_vertex_color;

out vec4 out_color;

vec3 lighting(vec3 material)
{
  vec3 light_color = vec3(1.);
  vec3 vertex_color = use_vertex_color ? f_vertex_color.rgb : vec3(0.);

  if(unlit)
  {
    light_color = vertex_color + (exterior_lit ? exterior_ambient_color : ambient_color);
  }
  else if(exterior_lit)
  {
    vec3 ambient = exterior_ambient_color + vertex_color.rgb;

    light_color = vec3(clamp (exterior_diffuse_color * max(dot(f_normal, exterior_light_dir), 0.0), 0.0, 1.0)) + ambient;
  }
  else
  {
    light_color = ambient_color + vertex_color.rgb;
  }  

  return material * light_color;
}

void main()
{
  float dist_from_camera = distance(camera, f_position);
  bool fog = draw_fog && !unfogged;

  if(fog && dist_from_camera >= fog_end)
  {
    out_color = vec4(fog_color, 1.);
    return;
  }

  vec4 tex = texture(tex1, f_texcoord);
  vec4 tex_2 = texture(tex2, f_texcoord_2);

  if(tex.a < alpha_test)
  {
    discard;
  }

  vec4 vertex_color = vec4(0., 0., 0., 1.f);
  vec3 light_color = vec3(1.);

  if(use_vertex_color) 
  {
    vertex_color = f_vertex_color;
  }


  // see: https://github.com/Deamon87/WebWowViewerCpp/blob/master/wowViewerLib/src/glsl/wmoShader.glsl
  if(shader_id == 3) // Env
  {
    vec3 env = tex_2.rgb * tex.rgb;
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 5) // EnvMetal
  {
    vec3 env = tex_2.rgb * tex.rgb * tex.a;
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 6) // TwoLayerDiffuse
  {
    vec3 layer2 = mix(tex.rgb, tex_2.rgb, tex_2.a);
    out_color = vec4(lighting(mix(layer2, tex.rgb, vertex_color.a)), 1.);
  }
  else // default shader, used for shader_id 0,1,2,4 (Diffuse, Specular, Metal, Opaque)
  {
    out_color = vec4(lighting(tex.rgb), 1.);
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
