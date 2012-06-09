// Brush.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef __BRUSH_H
#define __BRUSH_H

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

#endif
