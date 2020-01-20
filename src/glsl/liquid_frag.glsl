// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D texture;
uniform vec4 ocean_color_light;
uniform vec4 ocean_color_dark;
uniform vec4 river_color_light;
uniform vec4 river_color_dark;

uniform int type;
uniform float animtime;
uniform vec2 param;

in float depth_;
in vec2 tex_coord_;

out vec4 out_color;

vec2 rot2(vec2 p, float degree)
{
  float a = radians(degree);
  return mat2(cos(a), -sin(a), sin(a), cos(a))*p;
}

void main()
{
  // lava || slime
  if(type == 2 || type == 3)
  {
    out_color = texture2D (texture, tex_coord_ + vec2(param.x*animtime, param.y*animtime));
  }
  else
  {
    vec2 uv = rot2(tex_coord_ * param.x, param.y);
    vec4 texel = texture2D (texture, uv);
    vec4 lerp = (type == 1)
              ? mix (ocean_color_light, ocean_color_dark, depth_) 
              : mix (river_color_light, river_color_dark, depth_);
              
    vec4 tResult = clamp (texel + lerp, 0.0, 1.0); //clamp shouldn't be needed
    vec4 oColor = clamp (texel + tResult, 0.0, 1.0);
    out_color = vec4 (oColor.rgb, lerp.a);
  }  
}
