// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Brush.h>
#include <noggit/Log.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <noggit/texture_set.hpp>

#include <algorithm>    // std::min
#include <iostream>     // std::cout

#include <boost/utility/in_place_factory.hpp>

TextureSet::TextureSet (MapChunkHeader const& header, MPQFile* f, size_t base, MapTile* tile, bool big_alphamap, bool do_not_fix_alpha)
{
  nTextures = header.nLayers;
  for (int i = 0; i < 64; ++i)
  {
    const size_t array_index(i / 4);
    // it's a uint2 array so we need to read the bits as they come on the disk,
    // this means reading the highest bits from the uint8 first
    const size_t bit_index((3-((i) % 4)) * 2);

    _lod_texture_map.push_back(((header.low_quality_texture_map[array_index]) >> (bit_index)) & 3);
  }

  if (nTextures)
  {
    f->seek(base + header.ofsLayer + 8);

    for (size_t i = 0; i<nTextures; ++i)
    {
      f->read (&_layers_info[i], sizeof(ENTRY_MCLY));

      textures.emplace_back (tile->mTextureFilenames[_layers_info[i].textureID]);
    }

    size_t alpha_base = base + header.ofsAlpha + 8;

    for (unsigned int layer = 0; layer < nTextures; ++layer)
    {
      if (_layers_info[layer].flags & 0x100)
      {
        f->seek (alpha_base + _layers_info[layer].ofsAlpha);
        alphamaps[layer - 1] = boost::in_place (f, _layers_info[layer].flags, big_alphamap, do_not_fix_alpha);
      }
    }

    // always use big alpha for editing / rendering
    if (!big_alphamap)
    {
      convertToBigAlpha (true);
    }

    _need_amap_update = true;
  }
}

int TextureSet::addTexture(scoped_blp_texture_reference texture)
{
  int texLevel = -1;

  if (nTextures < 4U)
  {
    texLevel = nTextures;
    nTextures++;

    textures.emplace_back (texture);
    _layers_info[texLevel] = ENTRY_MCLY();

    if (texLevel)
    {
      alphamaps[texLevel - 1] = boost::in_place();
    }
  }
  
  _need_amap_update = true;

  return texLevel;
}

void TextureSet::switchTexture (scoped_blp_texture_reference oldTexture, scoped_blp_texture_reference newTexture)
{
  int texLevel = -1;
  for (size_t i = 0; i < nTextures; ++i)
  {
    if (textures[i] == oldTexture)
      texLevel = i;
    // prevent texture duplication
    if (textures[i] == newTexture)
      return;
  }

  if (texLevel != -1)
  {
    textures[texLevel] = newTexture;
  }
}

// swap 2 textures of a chunk with their alpha
// assume id1 < id2
void TextureSet::swapTexture(int id1, int id2)
{
  if (id1 >= 0 && id2 >= 0 && id1 < nTextures && id2 < nTextures)
  {
    std::swap(textures[id1], textures[id2]);

    int a1 = id1 - 1, a2 = id2 - 1;

    if (id1)
    {
      std::swap(alphamaps[a1], alphamaps[a2]);
    }
    else
    {
      uint8_t alpha[4096];
      
      for (int i = 0; i < 4096; ++i)
      {
        alpha[i] = 255 - sum_alpha(i);
      }

      alphamaps[a2]->setAlpha(alpha);
    }

    _need_amap_update = true;
  }
}

void TextureSet::eraseTextures()
{
  for (size_t i = nTextures-1; nTextures; --i)
  {
    eraseTexture(i);
  }

  _need_amap_update = true;
}

void TextureSet::eraseTexture(size_t id)
{
  if (id >= nTextures)
  {
    return;
  }    

  // shift textures above
  for (size_t i = id; i < nTextures - 1; i++)
  {
    if (i)
    {
      alphamaps[i - 1] = boost::none;
      std::swap (alphamaps[i - 1], alphamaps[i]);
    }

    _layers_info[i] = _layers_info[i + 1];
  }

  if (nTextures > 1)
  {
    alphamaps[nTextures - 2] = boost::none;
  }

  textures.erase(textures.begin()+id);
  nTextures--;

  _need_amap_update = true;
}

bool TextureSet::canPaintTexture(scoped_blp_texture_reference texture)
{
  if (nTextures)
  {
    for (size_t k = 0; k < nTextures; ++k)
    {
      if (textures[k] == texture)
      {
        return true;
      }
    }

    return nTextures < 4;
  }

  return true;
}

const std::string& TextureSet::filename(size_t id)
{
  return textures[id]->filename();
}

void TextureSet::bindTexture(size_t id, size_t activeTexture)
{
  opengl::texture::enable_texture (activeTexture);

  textures[id]->bind();
}

math::vector_2d TextureSet::anim_uv_offset(int id, int animtime) const
{
  uint32_t flags = _layers_info[id].flags;

  const int spd = (flags >> 3) & 0x7;
  const int dir = flags & 0x7;
  const float texanimxtab[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
  const float texanimytab[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
  const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];
  const int animspd = (const int)(200 * detail_size);
  float f = ((static_cast<int>(animtime*(spd / 7.0f))) % animspd) / static_cast<float>(animspd);
  return { f*fdx, f*fdy };
}

bool TextureSet::eraseUnusedTextures()
{
  if (nTextures < 2)
  {
    return false;
  }

  std::set<int> visible_tex;

  for (int i = 0; i < 4096 && visible_tex.size() < nTextures; ++i)
  {
    uint8_t sum = 0;
    for (int n = 0; n < nTextures - 1; ++n)
    {
      uint8_t a = alphamaps[n]->getAlpha(i);
      sum += a;
      if (a > 0)
      {
        visible_tex.emplace(n + 1);
      }
    }

    // base layer visible
    if (sum < 255)
    {
      visible_tex.emplace(0);
    }
  }

  if (visible_tex.size() < nTextures)
  {
    for (int i = nTextures-1; i >= 0; --i)
    {
      if (visible_tex.find(i) == visible_tex.end())
      {
        eraseTexture(i);
      }
    }

    _need_amap_update = true;
    return true;
  }

  return false;
}

int TextureSet::get_texture_index(scoped_blp_texture_reference texture, float target)
{
  for (int i = 0; i < nTextures; ++i)
  {
    if (textures[i] == texture)
    {
      return i;
    }
  }

  // don't add a texture for nothing
  if (target == 0)
  {
    return -1;
  }

  if (nTextures == 4 && !eraseUnusedTextures())
  {
    return -1;
  }

  return addTexture(texture);
}

// assume nTextures > 1
bool TextureSet::change_texture(int texture_id, size_t offset, uint strength, float pressure)
{
  auto set_alpha([&](int alpha_id, size_t offset, float value)
  {
    alphamaps[alpha_id]->setAlpha(offset, static_cast<uint8_t>(std::min(std::max(std::round(value), 0.0f), 255.0f)));
  });

  float current_alpha = texture_id ? alphamaps[texture_id - 1]->getAlpha(offset) : 255.f - sum_alpha(offset);
  float sum_other_alphas = 255.f - current_alpha;
  float alpha_change = (strength - current_alpha)* pressure;
  float alpha_after_change = current_alpha + alpha_change;

  if (current_alpha == strength)
  {
    return false;
  }

  // just change the first alphamap
  if (texture_id == 0 && current_alpha == 255.f)
  {
    set_alpha(0, offset, -alpha_change);
    return true;
  }

  // round alpha_change to be able to get to the extremes easier
  if (alpha_after_change < 1.f && alpha_change < 0.f)
  {
    alpha_change = -current_alpha;
  }
  else if (alpha_after_change > 254.f && alpha_change > 0.f)
  {
    alpha_change = (255.f - current_alpha);
  }

  for (int alpha_id = 0; alpha_id < nTextures - 1; ++alpha_id)
  {
    if (alpha_id + 1 == texture_id)
    {
      set_alpha(alpha_id, offset, current_alpha + alpha_change);
    }
    else
    {
      float a = alphamaps[alpha_id]->getAlpha(offset);
      set_alpha(alpha_id, offset, a - (a / sum_other_alphas) * alpha_change);
    }
  }  

  return true;
}

bool TextureSet::paintTexture(float xbase, float zbase, float x, float z, Brush* brush, uint strength, float pressure, scoped_blp_texture_reference texture)
{
  bool changed = false;

  float zPos, xPos, dist, radius;

  // hacky fix to make sure textures are blended between 2 chunks
  if (z < zbase)
  {
    zbase -= TEXDETAILSIZE;
  }
  else if (z > zbase + CHUNKSIZE)
  {
    zbase += TEXDETAILSIZE;
  }

  if (x < xbase)
  {
    xbase -= TEXDETAILSIZE;
  }
  else if (x > xbase + CHUNKSIZE)
  {
    xbase += TEXDETAILSIZE;
  }

  int tex_layer = get_texture_index(texture, strength);
  
  if (tex_layer == -1 || nTextures == 1)
  {
    return false;
  }

  radius = brush->getRadius();

  if (misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE) > radius)
  {
    return changed;
  }

  //First Lets find out do we have the texture already
  
  zPos = zbase;

  for (int j = 0; j < 64; j++)
  {
    xPos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      dist = misc::dist(x, z, xPos + TEXDETAILSIZE / 2.0f, zPos + TEXDETAILSIZE / 2.0f);
      xPos += TEXDETAILSIZE;

      if (dist>radius)
      {
        continue;
      }

      changed |= change_texture(tex_layer, i + 64 * j, strength, pressure*brush->getValue(dist));
    }
    zPos += TEXDETAILSIZE;
  }

  if (!changed)
  {
    return false;
  }

  // cleanup
  eraseUnusedTextures();

  _need_amap_update = true;

  return true;
}

bool TextureSet::replaceTexture(float xbase, float zbase, float x, float z, float radius, scoped_blp_texture_reference old_texture, scoped_blp_texture_reference new_texture)
{
  float dist = misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE);

  if (dist > radius)
  {
    return false;
  }

  bool changed = false;
  int old_tex_level = -1, new_tex_level = -1;
  float x_pos, z_pos = zbase;

  for (int i=0; i<nTextures; ++i)
  {
    if (textures[i] == old_texture)
    {
      old_tex_level = i;
    }
    if (textures[i] == new_texture)
    {
      new_tex_level = i;
    }
  }

  if (old_tex_level == -1 || (new_tex_level == -1 && nTextures == 4 && !eraseUnusedTextures()))
  {
    return false;
  }

  if (new_tex_level == -1)
  {
    new_tex_level = addTexture(new_texture);
  }

  for (int j = 0; j < 64; j++)
  {
    x_pos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      dist = misc::dist(x, z, x_pos + TEXDETAILSIZE / 2.0f, z_pos + TEXDETAILSIZE / 2.0f);
      int alpha_offset = j * 64 + i;

      if (dist <= radius)
      {
        if(old_tex_level == 0)
        {
          uint8_t base_alpha = 255.0f, new_tex_alpha = alphamaps[new_tex_level - 1]->getAlpha(alpha_offset);
          for (int n = 0; n < nTextures - 1; ++n)
          {
            base_alpha -= alphamaps[n]->getAlpha(alpha_offset);
          }

          alphamaps[new_tex_level - 1]->setAlpha(alpha_offset, base_alpha + new_tex_alpha);
        }
        else
        {
          if (new_tex_level != 0)
          {
            uint8_t old_tex_alpha = alphamaps[old_tex_level - 1]->getAlpha(alpha_offset);
            uint8_t new_tex_alpha = alphamaps[new_tex_level - 1]->getAlpha(alpha_offset);

            alphamaps[new_tex_level - 1]->setAlpha(alpha_offset, old_tex_alpha + new_tex_alpha);
          }

          alphamaps[old_tex_level - 1]->setAlpha(alpha_offset, 0);
        }

        changed = true;
      }

      x_pos += TEXDETAILSIZE;
    }

    z_pos += TEXDETAILSIZE;
  }

  if (changed)
  {
    _need_amap_update = true;
  }

  return changed;
}

size_t TextureSet::num()
{
  return nTextures;
}

unsigned int TextureSet::flag(size_t id)
{
  return _layers_info[id].flags;
}

unsigned int TextureSet::effect(size_t id)
{
  return _layers_info[id].effectID;
}

bool TextureSet::is_animated(std::size_t id) const
{
  return (id < nTextures ? (_layers_info[id].flags & FLAG_ANIMATE) : false);
}

void TextureSet::change_texture_flag(scoped_blp_texture_reference tex, std::size_t flag, bool add)
{
  for (size_t i = 0; i < nTextures; ++i)
  {
    if (textures[i] == tex)
    {
      if (add)
      {
        // override the current speed/rotation
        if (flag & 0x3F)
        {
          _layers_info[i].flags &= ~0x3F;
        }
        _layers_info[i].flags |= flag;
      }
      else
      {
        _layers_info[i].flags &= ~flag;
      }

      if (flag & FLAG_GLOW)
      {
        _layers_info[i].flags |= FLAG_GLOW;
      }
      else
      {
        _layers_info[i].flags &= ~FLAG_GLOW;
      }

      break;
    }
  }
}

uint8_t TextureSet::getAlpha(size_t id, size_t offset)
{
  return alphamaps[id]->getAlpha(offset);
}

const uint8_t *TextureSet::getAlpha(size_t id)
{
  return alphamaps[id]->getAlpha();
}

std::vector<std::vector<uint8_t>> TextureSet::save_alpha(bool big_alphamap)
{
  std::vector<std::vector<uint8_t>> amaps;

  if (nTextures > 1)
  {
    if (big_alphamap)
    {
      for (int i = 0; i < nTextures - 1; ++i)
      {
        const uint8_t* alphamap = alphamaps[i]->getAlpha();
        amaps.emplace_back(alphamap, alphamap + 4096);
      }
    }
    else
    {
      uint8_t tab[4096 * 3];
      alphas_to_old_alpha(tab);

      auto const combine_nibble
      (
        [&] (int layer, int pos)
        {
          int index = layer * 4096 + pos * 2;
          return ((tab[index] & 0xF0) >> 4) | (tab[index + 1] & 0xF0);
        }
      );

      for (size_t layer = 0; layer < nTextures - 1; ++layer)
      {
        amaps.emplace_back();
        for (int i = 0; i < 2048; ++i)
        {
          amaps.back().push_back(combine_nibble(layer, i));
        }
      }
    }    
  }

  return amaps;
}

scoped_blp_texture_reference TextureSet::texture(size_t id)
{
  return textures[id];
}

// dest = tab [4096 * (nTextures - 1)]
// call only if nTextures > 1
void TextureSet::alphas_to_big_alpha(uint8_t* dest)
{
  auto alpha 
  ( 
    [&] (int layer, int pos = 0)
    {
      return dest + layer * 4096 + pos;
    }
  );

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    memcpy(alpha(k), alphamaps[k]->getAlpha(), 4096);
  }

  for (int i = 0; i < 64 * 64; ++i)
  {
    float a = 1.f;

    for (int k = nTextures - 2; k >= 0 ; --k)
    {
      float f = static_cast<float>(*alpha(k, i)) * a;

      a -= (f / 255.f);

      *alpha(k, i) = static_cast<uint8_t>(std::round(f));
    }
  }
}

void TextureSet::convertToBigAlpha(bool loading)
{
  // nothing to do
  if (nTextures < 2)
  {
    return;
  }

  uint8_t tab[4096 * 3];

  alphas_to_big_alpha(tab);

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab + 4096 * k);
  }
}

// dest = tab [4096 * (nTextures - 1)]
// call only if nTextures > 1
void TextureSet::alphas_to_old_alpha(uint8_t* dest)
{
  auto alpha 
  ( 
    [&] (int layer, int pos = 0)
    {
      return dest + layer * 4096 + pos;
    }
  );

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    memcpy(alpha(k), alphamaps[k]->getAlpha(), 64 * 64);
  }

  for (int i = 0; i < 64 * 64; ++i)
  {
    // a = remaining visibility
    float a = 1.f;

    for (int k = nTextures - 2; k >= 0; --k)
    {
      if (a <= 0.f)
      {
        *alpha(k, i) = 0;
      }
      else
      {
        float current = static_cast<float>(*alpha(k, i));
        *alpha(k, i) = static_cast<uint8_t>(std::round(current / a));
        // current/255 = coef of visibility
        // = what need to remove to the remaining visibility
        a -= (current / 255.f);
      }      
    }
  }
}

void TextureSet::convertToOldAlpha()
{
  // nothing to do
  if (nTextures < 2)
  {
    return;
  }

  uint8_t tab[3 * 4096];

  alphas_to_old_alpha(tab);

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab + k * 4096);
  }

  _need_amap_update = true;
}

void TextureSet::merge_layers(size_t id1, size_t id2)
{
  if (id1 >= nTextures || id2 >= nTextures || id1 == id2)
  {
    throw std::invalid_argument("merge_layers: invalid layer id(s)");
  }

  if (id2 < id1)
  {
    std::swap(id2, id1);
  }

  // base alpha = 255 - sum_alpha(0..nTextures-1)
  // works only when alphamap are in the big alpha format (always the case when editing)
  if (id1 != 0)
  {
    uint8_t tab[2][64 * 64];

    memcpy(tab[0], alphamaps[id1 - 1]->getAlpha(), 64 * 64);
    memcpy(tab[1], alphamaps[id2 - 1]->getAlpha(), 64 * 64);

    for (int i = 0; i < 64 * 64; ++i)
    {
      tab[0][i] += tab[1][i];
    }

    alphamaps[id1-1]->setAlpha(tab[0]);
  }

  eraseTexture(id2);
  _need_amap_update = true;
}

bool TextureSet::removeDuplicate()
{
  bool changed = false;

  for (size_t i = 0; i < nTextures; i++)
  {
    for (size_t j = i + 1; j < nTextures; j++)
    {
      if (textures[i] == textures[j])
      {
        merge_layers(i, j);
        changed = true;
        j--; // otherwise it skips the next texture
      }
    }
  }

  return changed;
}

void TextureSet::bind_alpha(std::size_t id)
{
  opengl::texture::enable_texture (id);
  amap_gl_tex.bind();

  if (_need_amap_update)
  {
    std::vector<uint8_t> amap(3 * 64 * 64);

    if (nTextures)
    {
      uint8_t const* alpha_ptr[3];

      for (int i = 0; i < nTextures - 1; ++i)
      {
        alpha_ptr[i] = alphamaps[i]->getAlpha();
      }

      for (int i = 0; i < 64 * 64; ++i)
      {
        for (int alpha_id = 0; alpha_id < 3; ++alpha_id)
        {
          amap[i * 3 + alpha_id] = (alpha_id < nTextures - 1)
                                 ? *(alpha_ptr[alpha_id]++)
                                 : 0
                                 ;
        }
      }
    }    

    gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, amap.data());
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _need_amap_update = false;
  }
}

namespace
{
  misc::max_capacity_stack_vector<std::size_t, 4> current_layer_values
    (std::uint8_t nTextures, boost::optional<Alphamap> const* alphamaps, std::size_t pz, std::size_t px)
  {
    misc::max_capacity_stack_vector<std::size_t, 4> values (nTextures, 0xFF);
    for (std::uint8_t i = 1; i < nTextures; ++i)
    {
      values[i] = alphamaps[i - 1].get().getAlpha(64 * pz + px);
      values[0] -= values[i];
    }
    return values;
  }
}

std::vector<uint8_t> TextureSet::lod_texture_map()
{
  if (_need_lod_texture_map_update)
  {
    update_lod_texture_map();
  }

  return _lod_texture_map; 
}

void TextureSet::update_lod_texture_map()
{
  std::vector<std::uint8_t> lod;

  for (std::size_t z = 0; z < 8; ++z)
  {
    for (std::size_t x = 0; x < 8; ++x)
    {
      misc::max_capacity_stack_vector<std::size_t, 4> dominant_square_count (nTextures);

      for (std::size_t pz = z * 8; pz < (z + 1) * 8; ++pz)
      {
        for (std::size_t px = x * 8; px < (x + 1) * 8; ++px)
        {
          ++dominant_square_count[max_element_index (current_layer_values (nTextures, alphamaps.data(), pz, px))];
        }
      }
      lod.push_back (max_element_index (dominant_square_count));
    }
  }

  _lod_texture_map = lod;
  _need_lod_texture_map_update = false;
}

uint8_t TextureSet::sum_alpha(size_t offset) const
{
  uint8_t sum = 0;

  for (auto const& amap : alphamaps)
  {
    if (amap)
    {
      sum += amap->getAlpha(offset);
    }
  }

  return sum;
}

