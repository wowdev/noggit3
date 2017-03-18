// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/Log.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <opengl/context.hpp>
#include <opengl/matrix.hpp>
#include <opengl/scoped.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>

void liquid_render::draw ( std::function<void (opengl::scoped::use_program&)> actual
                         , math::vector_3d water_color_light
                         , math::vector_3d water_color_dark
                         , int animtime
                         )
{
  opengl::scoped::use_program water_shader {program};

  prepare_draw (water_shader, water_color_light, water_color_dark, animtime);

  actual (water_shader);
}

void liquid_render::prepare_draw ( opengl::scoped::use_program& water_shader
                                 , math::vector_3d water_color_light
                                 , math::vector_3d water_color_dark
                                 , int animtime
                                 )
{
  water_shader.uniform ("model_view", opengl::matrix::model_view());
  water_shader.uniform ("projection", opengl::matrix::projection());

  water_shader.uniform ("color_light", {water_color_light, 0.7f});
  water_shader.uniform ("color_dark", {water_color_dark, 0.9f});

  water_shader.sampler
    ( "texture"
    , GL_TEXTURE0
    , _textures[static_cast<std::size_t> (animtime / 60.0f) % _textures.size()].get()
    );
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
