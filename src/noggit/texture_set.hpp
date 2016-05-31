#pragma once

#include <stdint.h>

#include <noggit/mpq/file.h>
#include <noggit/alphamap.hpp>
#include <noggit/TextureManager.h>
#include "Brush.h"

namespace noggit
{
  static const double MAPCHUNK_RADIUS = 47.140452079103168293389624140323; //std::sqrt((533.33333/16)^2 + (533.33333/16)^2)

  class texture_set
  {
  public:
    typedef std::vector<std::string> texture_names_type;

    texture_set();
    ~texture_set();

    void initTextures(mpq::file* f, texture_names_type* texture_names, uint32_t size);
    void initAlphamaps(mpq::file* f, bool mBigAlpha, bool doNotFixAlpha);

    void bindTexture(size_t id, size_t activeTexture) const;
    void bindAlphamap(size_t id, size_t activeTexture) const;

    int addTexture(scoped_blp_texture_reference &texture);
    void eraseTextures();
    void switchTexture(scoped_blp_texture_reference &oldTexture, scoped_blp_texture_reference &newTexture);
    bool paintTexture(float xbase, float zbase, float x, float z, const brush& brush, float strength, float pressure, noggit::scoped_blp_texture_reference texture);

    const std::string filename(size_t id);

    const size_t num() const;
    const int animated(size_t id) const;
    const unsigned int flag(size_t id) const;
    const unsigned int effect(size_t id) const;

    void setAlpha(size_t id, size_t offset, unsigned char value);
    void setAlpha(size_t id, unsigned char *amap);

    const unsigned char getAlpha(size_t id, size_t offset);
    const unsigned char *getAlpha(size_t id);

    scoped_blp_texture_reference texture(size_t id);

  private:
    std::vector<scoped_blp_texture_reference> _textures;
    alphamap* _alphamaps[3];

    int _tex[4];
    int _animated[4];

    unsigned int _tex_flags[4];
    unsigned int _effect_id[4];
    unsigned int _mcal_offset[4];
  };
}
