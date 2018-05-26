// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <opengl/texture.hpp>

class Alphamap
{
public:
  Alphamap();
  Alphamap(MPQFile* f, unsigned int flags, bool mBigAlpha, bool doNotFixAlpha, bool upload_amap = true);

  void loadTexture();
  void upload();
  void bind();

  void setAlpha(size_t offset, unsigned char value);
  void setAlpha(unsigned char *pAmap);

  unsigned char getAlpha(size_t offset);
  const unsigned char *getAlpha();

  std::vector<uint8_t> compress() const;

private:
  void readCompressed(MPQFile *f);
  void readBigAlpha(MPQFile *f);
  void readNotCompressed(MPQFile *f, bool doNotFixAlpha);

  void createNew(); 

  uint8_t amap[64 * 64];
  opengl::texture map;
};
