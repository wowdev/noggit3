// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

// #define USEBLSFILES

#include <noggit/liquid_render.hpp>
#include <noggit/WMO.h>


struct LiquidVertex {
  unsigned char c[4];
  float h;
};

class wmo_liquid
{
public:
  wmo_liquid(MPQFile* f, WMOLiquidHeader const& header, WMOMaterial const& mat, bool indoor);
  
  void draw() { render->draw(); }

private:
  int initGeometry(MPQFile* f, opengl::call_list* draw_list);

  math::vector_3d pos;
  float texRepeats;
  bool mTransparency;
  int xtiles, ytiles;

  std::unique_ptr<liquid_render> render;
};