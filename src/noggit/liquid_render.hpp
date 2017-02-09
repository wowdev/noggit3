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
  bool _transparency;

  std::vector<scoped_blp_texture_reference> _textures;
};
