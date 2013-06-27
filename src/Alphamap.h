#ifndef ALPHAMAP_H
#define ALPHAMAP_H

#include <GL/glew.h>

#include "MPQ.h"

class Alphamap
{
public:
  Alphamap();
  Alphamap(MPQFile* f, unsigned int & flags, bool mBigAlpha);
  ~Alphamap();

  void setAlpha(size_t offset, unsigned char value);
  unsigned char getAlpha(size_t offset);
  void loadTexture();

  inline void bind()
  {
    glBindTexture(GL_TEXTURE_2D, map);
  }

  inline const GLuint id() const
  {
    return map;
  }

private:
  void readCompressed(MPQFile *f);
  void readBigAlpha(MPQFile *f);
  void readNotCompressed(MPQFile *f);

  void createNew();

  void genTexture();

  unsigned char amap[64*64];
  GLuint map;
};

#endif //ALPHAMAP_H
