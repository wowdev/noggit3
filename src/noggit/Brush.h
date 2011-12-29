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
  char tex[256*256];
  bool update;

public:
  void GenerateTexture();
  void setHardness( float H );
  void setRadius( float R );
  float getHardness();
  float getRadius();
  float getValue( float dist );
  opengl::texture* getTexture();
  bool needUpdate();
  void init();
};

#endif
