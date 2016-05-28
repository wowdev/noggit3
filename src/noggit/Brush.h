// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

namespace opengl
{
  class texture;
}

class brush
{
private:
  float hardness;
  float iradius;
  float oradius;
  float radius;
  opengl::texture* _texture;
  char tex[256 * 256];
  bool update;

public:
  void GenerateTexture();
  void setHardness(float H);
  void setRadius(float R);
  float getHardness() const;
  float getRadius() const;
  float getValue(float dist) const;
  opengl::texture* getTexture();
  bool needUpdate();
  brush(float r = 15.0f, float h = 0.5f);
};
