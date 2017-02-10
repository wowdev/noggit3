// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once


#include <noggit/MPQ.h>
#include <noggit/Video.h>
#include <noggit/TextureManager.h>
#include <opengl/call_list.hpp>
#include <opengl/shader.hpp>

#include <string>
#include <vector>
#include <memory>

class liquid_render
{
public:
  liquid_render(bool transparency = true, std::string const& filename = "");
  void draw (std::function<void (opengl::scoped::use_program&)>);

  void setTextures(std::string const& filename);
  void setTransparency(bool b) { _transparency = b; }

private:
  opengl::program const program
    { { GL_VERTEX_SHADER
      , R"code(
#version 110

attribute vec4 position;
attribute vec2 tex_coord;
attribute float depth;

uniform mat4 model_view;
uniform mat4 projection;

varying float depth_;
varying vec2 tex_coord_;

void main()
{
  depth_ = depth;
  tex_coord_ = tex_coord;

  gl_Position = projection * model_view * position;
}
)code"
      }
    , { GL_FRAGMENT_SHADER
      , R"code(
#version 110

uniform sampler2D texture;
uniform vec4 color_light;
uniform vec4 color_dark;
uniform float tex_repeat;

varying float depth_;
varying vec2 tex_coord_;

void main()
{
  vec4 texel = texture2D (texture, tex_coord_ / tex_repeat);
  vec4 lerp = mix (color_dark, color_light, depth_);
  vec4 tResult = clamp (texel + lerp, 0.0, 1.0); //clamp shouldn't be needed
  vec4 oColor = clamp (texel + tResult, 0.0, 1.0);
  gl_FragColor = vec4 (oColor.rgb, lerp.a);
}
)code"
      }
    };


  bool _transparency;

  std::vector<scoped_blp_texture_reference> _textures;
};
