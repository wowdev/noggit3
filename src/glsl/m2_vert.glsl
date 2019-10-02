// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 pos;
in vec3 normal;
in vec2 texcoord1;
in vec2 texcoord2;

#ifdef instanced
  in mat4 transform;
#else
  uniform mat4 transform;
#endif

out vec2 uv1;
out vec2 uv2;
out float camera_dist;
out vec3 norm;

uniform mat4 model_view;
uniform mat4 projection;

uniform int tex_unit_lookup_1;
uniform int tex_unit_lookup_2;

uniform mat4 tex_matrix_1;
uniform mat4 tex_matrix_2;

// code from https://wowdev.wiki/M2/.skin#Environment_mapping
vec2 sphere_map(vec3 vert, vec3 norm)
{
  vec3 normPos = -(normalize(vert));
  vec3 temp = (normPos - (norm * (2.0 * dot(normPos, norm))));
  temp = vec3(temp.x, temp.y, temp.z + 1.0);
 
  return ((normalize(temp).xy * 0.5) + vec2(0.5));
}

vec2 get_texture_uv(int tex_unit_lookup, vec3 vert, vec3 norm)
{
  if(tex_unit_lookup == 0)
  {
    return sphere_map(vert, norm);
  }
  else if(tex_unit_lookup == 1)
  {
    return (transpose(tex_matrix_1) * vec4(texcoord1, 0.0, 1.0)).xy;
  }
  else if(tex_unit_lookup == 2)
  {
    return (transpose(tex_matrix_2) * vec4(texcoord2, 0.0, 1.0)).xy;
  }
  else
  {
    return vec2(0.0);
  }
}

void main()
{
  vec4 vertex = model_view * transform * pos;

  // important to normalize because of the scaling !!
  norm = normalize(mat3(transform) * normal);

  uv1 = get_texture_uv(tex_unit_lookup_1, vertex.xyz, norm);
  uv2 = get_texture_uv(tex_unit_lookup_2, vertex.xyz, norm);

  camera_dist = -vertex.z;
  gl_Position = projection * vertex;
}
