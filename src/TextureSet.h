#ifndef TEXTURESET_H
#define TEXTURESET_H

#include "MPQ.h"
#include "Alphamap.h"
#include <stdint.h>

namespace OpenGL
{
  class Texture;
}

class Brush;
class MapTile;

class TextureSet
{
public:
  TextureSet();
  ~TextureSet();

  void initTextures(MPQFile* f, MapTile *maintile, uint32_t size);
  void initAlphamaps(MPQFile* f, size_t nLayers, bool mBigAlpha);

  void start2DAnim(int id);
  void stop2DAnim(int id);
  void startAnim(int id);
  void stopAnim(int id);

  void bindTexture(size_t id, size_t activeTexture);
  void bindAlphamap(size_t id, size_t activeTexture);

  int addTexture(OpenGL::Texture *texture);
  void eraseTextures();
  void switchTexture( OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture );
  bool paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, OpenGL::Texture* texture);

  const std::string& filename(size_t id);

  inline const size_t num()
  {
    return nTextures;
  }

  inline const unsigned int flag(size_t id)
  {
    return texFlags[id];
  }

  inline const unsigned int effect(size_t id)
  {
    return effectID[id];
  }

  inline void setAlpha(size_t id, size_t offset, unsigned char value)
  {
    alphamaps[id]->setAlpha(offset, value);
  }

  inline const unsigned char getAlpha(size_t id, size_t offset)
  {
    return alphamaps[id]->getAlpha(offset);
  }

  inline OpenGL::Texture* texture(size_t id)
  {
    return textures[id];
  }

private:
  OpenGL::Texture* textures[4];
  Alphamap* alphamaps[3];
  size_t nTextures;

  int tex[4];
  int animated[4];

  unsigned int texFlags[4];
  unsigned int effectID[4];
  unsigned int MCALoffset[4];
};

#endif //TEXTURESET_H
