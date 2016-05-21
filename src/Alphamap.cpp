#include "Alphamap.h"

Alphamap::Alphamap()
  : map(0)
{
  glGenTextures(1, &map);
  createNew();
  genTexture();
}

Alphamap::Alphamap(MPQFile *f, unsigned int &flags, bool mBigAlpha, bool doNotFixAlpha)
  : map(0)
{
  createNew();
  glGenTextures(1, &map);

  if(flags & 0x200 )
    readCompressed(f);
  else if(mBigAlpha)
    readBigAlpha(f);
  else
    readNotCompressed(f, doNotFixAlpha);

  genTexture();
}

Alphamap::~Alphamap()
{
  glDeleteTextures(1, &map);
}

void Alphamap::readCompressed(MPQFile *f)
{
  // compressed
  char* input = f->getPointer();

  for (std::size_t offset_output(0); offset_output < 4096;)
  {
    bool const fill(*input & 0x80);
    std::size_t const n(*input & 0x7F);
    ++input;

    if (fill)
    {
      memset(&amap[offset_output], *input, n);
      ++input;
    }
    else
    {
      memcpy(&amap[offset_output], input, n);
      input += n;
    }

    offset_output += n;
  }
}

void Alphamap::readBigAlpha(MPQFile *f)
{
  memcpy(amap, f->getPointer(), 64 * 64);
  f->seekRelative(0x1000);
}

void Alphamap::readNotCompressed(MPQFile *f, bool doNotFixAlpha)
{
  // not compressed
  unsigned char *p;
  char *abuf = f->getPointer();
  p = amap;

  for (std::size_t x(0); x < 64; ++x)
  {
    for (std::size_t y(0); y < 64; y += 2)
    {
      amap[x * 64 + y + 0] = ((*abuf & 0x0f) << 4) | (*abuf & 0x0f);
      amap[x * 64 + y + 1] = ((*abuf & 0xf0) >> 4) | (*abuf & 0xf0);
      ++abuf;
    }
  }

  if (doNotFixAlpha)
  {
    for (std::size_t i(0); i < 64; ++i)
    {
      amap[i * 64 + 63] = amap[i * 64 + 62];
      amap[63 * 64 + i] = amap[62 * 64 + i];
    }
    amap[63 * 64 + 63] = amap[62 * 64 + 62];
  }
  f->seekRelative(0x800);
}

void Alphamap::createNew()
{
  memset(amap, 0, 64 * 64);
}

void Alphamap::loadTexture()
{
  glBindTexture(GL_TEXTURE_2D, map);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap);
}

void Alphamap::genTexture()
{
  loadTexture();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Alphamap::bind()
{
  glBindTexture(GL_TEXTURE_2D, map);
}

bool Alphamap::isValid()
{
  return (map > 0);
}

void Alphamap::setAlpha(size_t offset, unsigned char value)
{
  amap[offset] = value;
}

void Alphamap::setAlpha(unsigned char *pAmap)
{
  memcpy(amap, pAmap, 64*64);
}

const unsigned char Alphamap::getAlpha(size_t offset)
{
  return amap[offset];
}

const unsigned char *Alphamap::getAlpha()
{
  return amap;
}


