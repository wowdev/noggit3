// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once


#include <noggit/MPQ.h>
#include <noggit/TextureManager.h>
#include <opengl/shader.hpp>

#include <string>
#include <vector>
#include <memory>

class liquid_render
{
public:
  liquid_render() = default;
  void prepare_draw ( opengl::scoped::use_program& water_shader
                    , int liquid_id
                    , int animtime
                    );

  opengl::program const& shader_program() const
  {
    return program;
  }

  void force_texture_update();
  std::size_t get_texture_index(int liquid_id, int animtime) const;

private:
  void add_liquid_id(int liquid);

  boost::optional<int> _current_liquid_id;
  int _current_anim_time = 0;

  opengl::program program
    { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("liquid_vs") }
    , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("liquid_fs") }
    };

  std::map<int, int> _liquid_id_types;
  std::map<int, math::vector_2d> _float_param_by_liquid_id;
  std::map<int, std::vector<scoped_blp_texture_reference>> _textures_by_liquid_id;
};
