// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 position;
in vec3 normal;
in vec4 vertex_color;
in vec2 texcoord;
in vec2 texcoord_2;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_texcoord;
out vec2 f_texcoord_2;
out vec4 f_vertex_color;

uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

uniform int shader_id;

void main()
{
  vec4 pos = transform * position;
  vec4 view_space_pos = model_view * pos;
  gl_Position = projection * view_space_pos;

  f_position = pos.xyz;
  f_normal = mat3(transform) * normal;

  // Env and EnvMetal
  if(shader_id == 3 || shader_id == 5)
  {
    f_texcoord = texcoord;
    f_texcoord_2 = reflect(normalize(view_space_pos.xyz), f_normal).xy;
  }
  else
  {
    f_texcoord = texcoord;
    f_texcoord_2 = texcoord_2;
  }
  
  f_vertex_color = vertex_color;
}
