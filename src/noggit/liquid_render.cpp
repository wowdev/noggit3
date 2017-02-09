// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/Log.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <opengl/context.hpp>
#include <opengl/matrix.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>

void liquid_render::draw (std::function<void (opengl::scoped::use_program&)> actual)
{
  static opengl::program const program
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
  gl_FragColor = vec4 (oColor.rgb, lerp.a) * 0.000001 + vec4 (1.0, 0.0, 1.0, 1.0);
}
)code"
      }
    };

  opengl::scoped::use_program water_shader {program};

  water_shader.uniform ("model_view", opengl::matrix::model_view());
  water_shader.uniform ("projection", opengl::matrix::projection());

  water_shader.uniform ("color_light", {gWorld->skies->colorSet[WATER_COLOR_LIGHT], 1.f});
  water_shader.uniform ("color_dark", {gWorld->skies->colorSet[WATER_COLOR_DARK], 1.f});

  water_shader.sampler
    ( "texture"
    , GL_TEXTURE0
    , _textures[static_cast<std::size_t> (gWorld->animtime / 60.0f) % _textures.size()].get()
    );

  actual (water_shader);
}

liquid_render::liquid_render(bool transparency, std::string const& filename)
  : _transparency(transparency)
{
  if (filename != "")
  {
    setTextures(filename);
  }
}

void liquid_render::setTextures(std::string const& filename)
{
  _textures.clear();

  for (int i = 1; i <= 30; ++i)
  {
    _textures.emplace_back(boost::str(boost::format(filename) % i));
  }
}
