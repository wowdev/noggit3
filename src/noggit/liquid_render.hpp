// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once


#include <noggit/MPQ.h>
#ifdef USEBLSFILES
#include <noggit/Shaders.h>
#endif
#include <noggit/Video.h>
#include <noggit/TextureManager.h>

#include <string>
#include <vector>
#include <memory>

namespace opengl
{
  class call_list;
}


void loadWaterShader();

class liquid_render
{
public:
  liquid_render(bool transparency, std::string const& filename = "", opengl::call_list* draw_list = nullptr);
  void draw();

  void setTextures(std::string const& filename);
  void changeDrawList(opengl::call_list* draw_list);

private:
  bool _ready;
  bool _transparency;
  

  std::unique_ptr<opengl::call_list> _draw_list;
  std::vector<scoped_blp_texture_reference> _textures;
};
