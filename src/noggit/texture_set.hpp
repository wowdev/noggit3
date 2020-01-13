// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/MPQ.h>
#include <noggit/alphamap.hpp>
#include <noggit/MapHeaders.h>

#include <cstdint>
#include <array>

class Brush;
class MapTile;

class TextureSet
{
public:
  TextureSet() = delete;
  TextureSet(MapChunkHeader const& header, MPQFile* f, size_t base, MapTile* tile, bool big_alphamap, bool do_not_fix_alpha);

  math::vector_2d anim_uv_offset(int id, int animtime) const;

  void bindTexture(size_t id, size_t activeTexture);

  int addTexture(scoped_blp_texture_reference texture);
  void eraseTexture(size_t id);
  void eraseTextures();
  // return true if at least 1 texture has been erased
  bool eraseUnusedTextures();
  void swapTexture(int id1, int id2);
  void switchTexture(scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture);
  bool paintTexture(float xbase, float zbase, float x, float z, Brush* brush, uint strength, float pressure, scoped_blp_texture_reference texture);
  bool replaceTexture(float xbase, float zbase, float x, float z, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture);
  bool canPaintTexture(scoped_blp_texture_reference const& texture);

  const std::string& filename(size_t id);

  size_t const& num() const { return nTextures; }
  unsigned int flag(size_t id);
  unsigned int effect(size_t id);
  bool is_animated(std::size_t id) const;
  void change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add);

  uint8_t getAlpha(size_t id, size_t offset);
  const uint8_t *getAlpha(size_t id);

  std::vector<std::vector<uint8_t>> save_alpha(bool big_alphamap);

  void convertToBigAlpha();
  void convertToOldAlpha();

  void merge_layers(size_t id1, size_t id2);
  bool removeDuplicate();

  scoped_blp_texture_reference texture(size_t id);

  void bind_alpha(std::size_t id);

  std::vector<uint8_t> lod_texture_map();

private:
  int get_texture_index_or_add (scoped_blp_texture_reference texture, float target);
  bool change_texture(int texture_id, size_t offset, uint strength, float pressure);

  uint8_t sum_alpha(size_t offset) const;

  void alphas_to_big_alpha(uint8_t* dest);
  void alphas_to_old_alpha(uint8_t* dest);

  void update_lod_texture_map();

  std::vector<scoped_blp_texture_reference> textures;
  std::array<boost::optional<Alphamap>, 3> alphamaps;
  opengl::texture amap_gl_tex;
  bool _need_amap_update = true;
  size_t nTextures;
  
  std::vector<uint8_t> _lod_texture_map;
  bool _need_lod_texture_map_update = false;

  ENTRY_MCLY _layers_info[4];
};
