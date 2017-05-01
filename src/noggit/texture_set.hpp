// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/MPQ.h>
#include <noggit/alphamap.hpp>

#include <cstdint>
#include <array>

class Brush;
class MapTile;

class TextureSet
{
public:
  void initTextures(MPQFile* f, MapTile *maintile, uint32_t size);
  void initAlphamaps(MPQFile* f, size_t nLayers, bool mBigAlpha, bool doNotFixAlpha);

  void startAnim(int id, int animtime);
  void stopAnim(int id);

  void bindTexture(size_t id, size_t activeTexture);
  void bindAlphamap(size_t id, size_t activeTexture);

  int addTexture(scoped_blp_texture_reference texture);
  void eraseTexture(size_t id);
  void eraseTextures();
  // return true if at least 1 texture has been erased
  bool eraseUnusedTextures();
  void swapTexture(int id1, int id2);
  void switchTexture(scoped_blp_texture_reference oldTexture, scoped_blp_texture_reference newTexture);
  bool paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool canPaintTexture(scoped_blp_texture_reference texture);

  const std::string& filename(size_t id);

  size_t num();
  unsigned int flag(size_t id);
  unsigned int effect(size_t id);
  bool is_animated(std::size_t id) const;
  void change_texture_flag(scoped_blp_texture_reference tex, std::size_t flag, bool add);

  void setAlpha(size_t id, size_t offset, unsigned char value);
  void setAlpha(size_t id, unsigned char *amap);

  unsigned char getAlpha(size_t id, size_t offset);
  const unsigned char *getAlpha(size_t id);

  std::vector<std::vector<char>> get_compressed_alphamaps();

  void convertToBigAlpha();
  void convertToOldAlpha();

  void mergeAlpha(size_t id1, size_t id2);
  bool removeDuplicate();

  scoped_blp_texture_reference texture(size_t id);

  void bind_alpha(std::size_t id);

private:
  void alphas_to_big_alpha(unsigned char* dest);
  std::vector<char> get_compressed_alpha(std::size_t id, unsigned char* alphas);
  void regen_alpha_tex();

  std::vector<scoped_blp_texture_reference> textures;
  std::array<boost::optional<Alphamap>, 3> alphamaps;
  std::vector<unsigned char> alphamap_tex;
  opengl::texture amap_gl_tex;
  size_t nTextures;

  int tex[4];

  unsigned int texFlags[4];
  unsigned int effectID[4];
  unsigned int MCALoffset[4];
};
