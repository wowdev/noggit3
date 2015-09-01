#include "Alphamap.h"

Alphamap::Alphamap()
  : map(0)
{
  glGenTextures(1, &map);
  createNew();
  genTexture();
}

Alphamap::Alphamap(MPQFile *f, unsigned int &flags, bool mBigAlpha)
  : map(0)
{
  createNew();
  glGenTextures(1, &map);

  if(flags & 0x200 )
    readCompressed(f);
  else if(mBigAlpha)
    readBigAlpha(f);
  else
    readNotCompressed(f);

  genTexture();
}

Alphamap::~Alphamap()
{
  glDeleteTextures(1, &map);
}

void Alphamap::readCompressed(MPQFile *f)
{
  // compressed
  // 21-10-2008 by Flow
  unsigned offI = 0; //offset IN buffer
  unsigned offO = 0; //offset OUT buffer
  char* buffIn = f->getPointer(); // pointer to data in adt file

  while( offO < 4096 )
  {
    // fill or copy mode
	  bool fill = (buffIn[offI] & 0x80) != 0;
    unsigned n = buffIn[offI] & 0x7F;
    offI++;
    for( unsigned k = 0; k < n; ++k )
    {
      if (offO == 4096) break;
      amap[offO] = buffIn[offI];
      offO++;
      if( !fill )
        offI++;
    }
    if( fill ) offI++;
  }
}

void Alphamap::readBigAlpha(MPQFile *f)
{
  // not compressed
  unsigned char *p;
  char *abuf = f->getPointer();
  p = amap;
  for (int j=0; j<64; ++j) {
    for (int i=0; i<64; ++i) {
      *p++ = *abuf++;
    }

  }
  memcpy(amap+63*64,amap+62*64,64);
  f->seekRelative(0x1000);
}

void Alphamap::readNotCompressed(MPQFile *f)
{
  // not compressed
  unsigned char *p;
  char *abuf = f->getPointer();
  p = amap;
  for (int j=0; j<63; ++j) {
    for (int i=0; i<32; ++i) {
      unsigned char c = *abuf++;
      *p++ = static_cast<unsigned char>((255*(static_cast<int>(c & 0x0f)))/0x0f);
      if(i != 31)
      {
        *p++ = static_cast<unsigned char>((255*(static_cast<int>(c & 0xf0)))/0xf0);
      }
      else
      {
        *p++ = static_cast<unsigned char>((255*(static_cast<int>(c & 0x0f)))/0x0f);
      }
    }

  }
  memcpy(amap+63*64,amap+62*64,64);
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


