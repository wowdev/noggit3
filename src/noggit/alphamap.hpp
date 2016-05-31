#pragma once

#include <noggit/mpq/file.h>
#include <opengl/types.h>

namespace noggit
{
  class alphamap
  {
  public:
    alphamap();
    alphamap(mpq::file* f, unsigned int & flags, bool mBigAlpha, bool doNotFixAlpha);
    ~alphamap();

    void loadTexture();

    void bind();
    bool isValid();

    void setAlpha(size_t offset, unsigned char value);
    void setAlpha(unsigned char *pAmap);

    const unsigned char getAlpha(size_t offset);
    const unsigned char *getAlpha();

  private:
    void readCompressed(mpq::file *f);
    void readBigAlpha(mpq::file *f);
    void readNotCompressed(mpq::file *f, bool doNotFixAlpha);

    void genTexture();

    unsigned char amap[64 * 64];
    GLuint map;
  };
}
