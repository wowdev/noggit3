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

void liquid_render::draw_wmo ( std::function<void (opengl::scoped::use_program&)> actual
                             , math::vector_3d water_color_light
                             , math::vector_3d water_color_dark
                             , int liquid_id
                             , int animtime
                             )
{
  opengl::scoped::use_program water_shader {program};

  water_shader.uniform ("model_view", opengl::matrix::model_view());
  water_shader.uniform ("projection", opengl::matrix::projection());

  water_shader.uniform ("color_light", water_color_light);
  water_shader.uniform ("color_dark",  water_color_dark);

  prepare_draw (water_shader, liquid_id, animtime);
  actual (water_shader);
}

void liquid_render::prepare_draw ( opengl::scoped::use_program& water_shader
                                 , int liquid_id
                                 , int animtime
                                 )
{

  if (_current_anim_time != animtime || liquid_id != _current_liquid_id)
  {
    _current_anim_time = animtime;
    _current_liquid_id = liquid_id;

    if (_textures_by_liquid_id[liquid_id].empty())
    {
      add_liquid_id(liquid_id);
    }

    auto const& textures = _textures_by_liquid_id[liquid_id];

    water_shader.sampler
    ("texture"
      , GL_TEXTURE0
      , textures[static_cast<std::size_t> (animtime / 60.0f) % textures.size()].get()
    );
  }
  
}

void liquid_render::add_liquid_id(int liquid_id)
{
  auto& textures = _textures_by_liquid_id[liquid_id];
  textures.clear();

  std::string filename;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(liquid_id);

    // fix to now crash when using procedural water (id 100)
    if (lLiquidTypeRow.getInt(LiquidTypeDB::ShaderType) == 3)
    {
      filename = "XTextures\\river\\lake_a.%d.blp";
    }
    else
    {
      filename = lLiquidTypeRow.getString(LiquidTypeDB::TextureFilenames);
    }
  }
  catch (...)
  {
    // Fallback, when there is no information.
    filename = "XTextures\\river\\lake_a.%d.blp";
  }

  for (int i = 1; i <= 30; ++i)
  {
    try 
    {
      textures.emplace_back(boost::str(boost::format(filename) % i));
    }
    catch (...)
    {
      break;
    }    
  }
}

