// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/texture.hpp>

#include <memory>

class Brush
{
private:
  float hardness;
  float iradius;
  float oradius;
  float radius;
  std::unique_ptr<opengl::texture> _texture;
  char tex[256 * 256];
  bool update;

public:
  void GenerateTexture();
  void setHardness(float H);
  void setRadius(float R);
  float getHardness();
  float getRadius();
  float getValue(float dist);
  opengl::texture* getTexture();
  bool needUpdate();
  void init();
};
