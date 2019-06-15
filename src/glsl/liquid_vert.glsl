// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 position;
in vec2 tex_coord;
in float depth;

uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

uniform int use_transform = int(0);

out float depth_;
out vec2 tex_coord_;

void main()
{
  depth_ = depth;
  tex_coord_ = tex_coord;

  if(use_transform == 1)
  {
    gl_Position = projection * model_view * transform * position;
  }
  else
  {
    gl_Position = projection * model_view * position;
  }
}
