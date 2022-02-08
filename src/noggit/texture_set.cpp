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

TextureSet::TextureSet (MapChunkHeader const& header, MPQFile* f, size_t base, MapTile* tile, bool use_big_alphamaps, bool do_not_fix_alpha_map, bool do_not_convert_alphamaps)
  : nTextures(header.nLayers)
  , _do_not_convert_alphamaps(do_not_convert_alphamaps)
{
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
        alphamaps[layer - 1] = boost::in_place (f, _layers_info[layer].flags, use_big_alphamaps, do_not_fix_alpha_map);
      }
    }

    // always use big alpha for editing / rendering
    if (!use_big_alphamaps && !_do_not_convert_alphamaps)
    {
      convertToBigAlpha();
    }

    _need_amap_update = true;
  }
}

int TextureSet::addTexture (scoped_blp_texture_reference texture)
{
  int texLevel = -1;

  if (nTextures < 4U)
  {
    texLevel = nTextures;
    nTextures++;

    textures.emplace_back (std::move (texture));
    _layers_info[texLevel] = ENTRY_MCLY();

    if (texLevel)
    {
      alphamaps[texLevel - 1] = boost::in_place();
    }

    if (tmp_edit_values && nTextures == 1)
    {
      tmp_edit_values.get()[0].fill(255.f);
    }
  }

  _need_amap_update = true;
  _need_lod_texture_map_update = true;

  return texLevel;
}

void TextureSet::replace_texture (scoped_blp_texture_reference const& texture_to_replace, scoped_blp_texture_reference replacement_texture)
{
  int texture_to_replace_level = -1, replacement_texture_level = -1;

  for (size_t i = 0; i < nTextures; ++i)
  {
    if (textures[i] == texture_to_replace)
    {
      texture_to_replace_level = i;
    }
    else if (textures[i] == replacement_texture)
    {
      replacement_texture_level = i;
    }
  }

  if (texture_to_replace_level != -1)
  {
    textures[texture_to_replace_level] = std::move (replacement_texture);

    // prevent texture duplication
    if (replacement_texture_level != -1 && replacement_texture_level != texture_to_replace_level)
    {
      // temp alphamap changes are applied in here
      merge_layers(texture_to_replace_level, replacement_texture_level);
    }
  }
}

void TextureSet::swap_layers(int layer_1, int layer_2)
{
  int lower_texture_id = std::min(layer_1, layer_2);
  int upper_texture_id = std::max(layer_1, layer_2);

  if (lower_texture_id == upper_texture_id)
  {
    return;
  }

  if (lower_texture_id > upper_texture_id)
  {
    std::swap(lower_texture_id, upper_texture_id);
  }

  if (lower_texture_id >= 0 && upper_texture_id >= 0 && lower_texture_id < nTextures && upper_texture_id < nTextures)
  {
    apply_alpha_changes();

    std::swap(textures[lower_texture_id], textures[upper_texture_id]);
    std::swap(_layers_info[lower_texture_id], _layers_info[upper_texture_id]);

    int a1 = lower_texture_id - 1, a2 = upper_texture_id - 1;

    if (lower_texture_id)
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
    _need_lod_texture_map_update = true;
  }
}

void TextureSet::eraseTextures()
{
  if (nTextures == 0)
  {
    return;
  }

  textures.clear();

  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
    {
      alphamaps[i - 1] = boost::none;
    }
    _layers_info[i] = ENTRY_MCLY();
  }

  nTextures = 0;

  _lod_texture_map.resize(8 * 8);
  memset(_lod_texture_map.data(), 0, 64 * sizeof(std::uint8_t));

  _need_amap_update = true;
  _need_lod_texture_map_update = true;

  tmp_edit_values = boost::none;
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

    if (tmp_edit_values)
    {
      tmp_edit_values.get()[i].swap(tmp_edit_values.get()[i+1]);
    }

    _layers_info[i] = _layers_info[i + 1];
  }

  if (nTextures > 1)
  {
    alphamaps[nTextures - 2] = boost::none;
  }

  textures.erase(textures.begin()+id);
  nTextures--;

  // erase the old info as a precaution but it's overriden when adding a new texture
  _layers_info[nTextures] = ENTRY_MCLY();

  // set the default values for the temporary alphamap too
  if (tmp_edit_values)
  {
    tmp_edit_values.get()[nTextures].fill(0.f);
  }

  _need_amap_update = true;
  _need_lod_texture_map_update = true;
}

bool TextureSet::canPaintTexture(scoped_blp_texture_reference const& texture)
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
  return textures[id]->filename;
}

void TextureSet::bindTexture(size_t id, size_t activeTexture, std::vector<int>& textures_bound)
{
  if (textures_bound[id] != textures[id].blp_id())
  {
    opengl::texture::set_active_texture(activeTexture);
    textures[id]->bind();

    textures_bound[id] = textures[id].blp_id();
  }
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

  if (tmp_edit_values)
  {
    auto& amaps = tmp_edit_values.get();

    for (int i = 0; i < 4096 && visible_tex.size() < nTextures; ++i)
    {
      for (int layer = 0; layer < nTextures; ++layer)
      {
        if (amaps[layer][i] > 0.f)
        {
          visible_tex.emplace(i);
        }
      }
    }
  }
  else
  {
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
  }

  if (visible_tex.size() < nTextures)
  {
    for (int i = nTextures - 1; i >= 0; --i)
    {
      if (visible_tex.find(i) == visible_tex.end())
      {
        eraseTexture(i);
      }
    }

    _need_amap_update = true;
    _need_lod_texture_map_update = true;
    return true;
  }

  return false;
}

int TextureSet::get_texture_index_or_add (scoped_blp_texture_reference texture, float target)
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

  return addTexture (std::move (texture));
}

bool TextureSet::paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  bool changed = false;

  float zPos, xPos, dist, radius;

  int tex_layer = get_texture_index_or_add (std::move (texture), strength);

  if (tex_layer == -1 || nTextures == 1)
  {
    return nTextures == 1;
  }

  radius = brush->getRadius();

  if (misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE) > radius)
  {
    return changed;
  }

  create_temporary_alphamaps_if_needed();
  auto& amaps = tmp_edit_values.get();

  zPos = zbase;

  for (int j = 0; j < 64; j++)
  {
    xPos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      dist = misc::dist(x, z, xPos + TEXDETAILSIZE / 2.0f, zPos + TEXDETAILSIZE / 2.0f);

      if (dist <= radius)
      {
        // use double for more precision
        std::array<double,4> alpha_values;
        double total = 0.;

        for (int n = 0; n < 4; ++n)
        {
          total += alpha_values[n] = amaps[n][i + 64 * j];
        }

        double current_alpha = alpha_values[tex_layer];
        double sum_other_alphas = (total - current_alpha);
        double alpha_change = (strength - current_alpha) * pressure * brush->getValue(dist);

        // alpha too low, set it to 0 directly
        if (alpha_change < 0. && current_alpha + alpha_change < 1.)
        {
          alpha_change = -current_alpha;
        }

        if (!misc::float_equals(current_alpha, strength))
        {
          if (sum_other_alphas < 1.)
          {
            // alpha is currently at 254/255 -> set it at 255 and clear the rest of the values
            if (alpha_change > 0.f)
            {
              for (int layer = 0; layer < nTextures; ++layer)
              {
                alpha_values[layer] = layer == tex_layer ? 255. : 0.f;
              }
            }
            // all the other textures amount for less an 1/255 -> add the alpha_change (negative) to current texture and remove it from the first non current texture, clear the rest
            else
            {
              bool change_applied = false;

              for (int layer = 0; layer < nTextures; ++layer)
              {
                if (layer == tex_layer)
                {
                  alpha_values[layer] += alpha_change;
                }
                else
                {
                  if (!change_applied)
                  {
                    alpha_values[layer] -= alpha_change;
                  }
                  else
                  {
                    alpha_values[tex_layer] += alpha_values[layer];
                    alpha_values[layer] = 0.;
                  }

                  change_applied = true;
                }
              }
            }
          }
          else
          {
            for (int layer = 0; layer < nTextures; ++layer)
            {
              if (layer == tex_layer)
              {
                alpha_values[layer] += alpha_change;
              }
              else
              {
                alpha_values[layer] -= alpha_change * alpha_values[layer] / sum_other_alphas;

                // clear values too low to be visible
                if (alpha_values[layer] < 1.)
                {
                  alpha_values[tex_layer] += alpha_values[layer];
                  alpha_values[layer] = 0.f;
                }
              }
            }
          }

          double total_final = std::accumulate(alpha_values.begin(), alpha_values.end(), 0.);

          // failsafe in case the sum of all alpha values deviate
          if (std::abs(total_final - 255.) > 0.001)
          {
            for (double& d : alpha_values)
            {
              d = d * 255. / total_final;
            }
          }

          for (int n = 0; n < 4; ++n)
          {
            amaps[n][i + 64 * j] = static_cast<float>(alpha_values[n]);
          }

          changed = true;
        }
      }

      xPos += TEXDETAILSIZE;
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
  _need_lod_texture_map_update = true;

  return true;
}

bool TextureSet::replace_texture( float xbase
                                , float zbase
                                , float x
                                , float z
                                , float radius
                                , scoped_blp_texture_reference const& texture_to_replace
                                , scoped_blp_texture_reference replacement_texture
                                )
{
  float dist = misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE);

  if (dist > radius)
  {
    return false;
  }

  // if the chunk is fully inside the brush, just swap the 2 textures
  if (misc::square_is_in_circle(x, z, radius, xbase, zbase, CHUNKSIZE))
  {
    replace_texture(texture_to_replace, std::move (replacement_texture));
    return true;
  }

  bool changed = false;
  int old_tex_level = -1, new_tex_level = -1;
  float x_pos, z_pos = zbase;

  for (int i=0; i<nTextures; ++i)
  {
    if (textures[i] == texture_to_replace)
    {
      old_tex_level = i;
    }
    if (textures[i] == replacement_texture)
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
    new_tex_level = addTexture(std::move (replacement_texture));
  }

  if (old_tex_level == new_tex_level)
  {
    return false;
  }

  create_temporary_alphamaps_if_needed();
  auto& amap = tmp_edit_values.get();

  for (int j = 0; j < 64; j++)
  {
    x_pos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      dist = misc::dist(x, z, x_pos + TEXDETAILSIZE / 2.0f, z_pos + TEXDETAILSIZE / 2.0f);

      if (dist <= radius)
      {
        int offset = j * 64 + i;

        amap[new_tex_level][offset] += amap[old_tex_level][offset];
        amap[old_tex_level][offset] = 0.f;

        changed = true;
      }

      x_pos += TEXDETAILSIZE;
    }

    z_pos += TEXDETAILSIZE;
  }

  if (changed)
  {
    _need_amap_update = true;
    _need_lod_texture_map_update = true;
  }

  return changed;
}

unsigned int TextureSet::flag(size_t id)
{
  return _layers_info[id].flags;
}

void TextureSet::setEffect(size_t id, int value)
{
  _layers_info[id].effectID = value;
}

unsigned int TextureSet::effect(size_t id)
{
  return _layers_info[id].effectID;
}

bool TextureSet::is_animated(std::size_t id) const
{
  return (id < nTextures ? (_layers_info[id].flags & FLAG_ANIMATE) : false);
}

void TextureSet::change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add)
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

std::vector<std::vector<uint8_t>> TextureSet::save_alpha(bool big_alphamap)
{
  std::vector<std::vector<uint8_t>> amaps;

  apply_alpha_changes();

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

      if (_do_not_convert_alphamaps)
      {
        for (size_t k = 0; k < nTextures - 1; k++)
        {
          memcpy(tab + (k*64*64), alphamaps[k]->getAlpha(), 64 * 64);
        }
      }
      else
      {
        alphas_to_old_alpha(tab);
      }
      

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
        amaps.emplace_back(2048);
        auto& layer_data = amaps.back();

        for (int i = 0; i < 2048; ++i)
        {
          layer_data[i] = combine_nibble(layer, i);
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
    int a = 255;

    for (int k = nTextures - 2; k >= 0; --k)
    {
      uint8_t val = misc::rounded_255_int_div(*alpha(k, i) * a);
      a -= val;
      *alpha(k, i) = val;
    }
  }
}

void TextureSet::convertToBigAlpha()
{
  // nothing to do
  if (nTextures < 2)
  {
    return;
  }

  uint8_t tab[4096 * 3];

  apply_alpha_changes();
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
    int a = 255;

    for (int k = nTextures - 2; k >= 0; --k)
    {
      if (a <= 0)
      {
        *alpha(k, i) = 0;
      }
      else
      {
        int current = *alpha(k, i);
        // convert big alpha value to old alpha
        *alpha(k, i) = misc::rounded_int_div(current * 255, a);
        // remove big alpha value from the remaining visibility
        a -= current;
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

  apply_alpha_changes();
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

  create_temporary_alphamaps_if_needed();

  auto& amap = tmp_edit_values.get();

  for (int i = 0; i < 64 * 64; ++i)
  {
    amap[id1][i] += amap[id2][i];
    // no need to set the id alphamap to 0, it'll be done in "eraseTexture(id2)"
  }

  eraseTexture(id2);
  _need_amap_update = true;
  _need_lod_texture_map_update = true;
}

bool TextureSet::removeDuplicate()
{
  bool changed = apply_alpha_changes();

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
  opengl::texture::set_active_texture(id);
  amap_gl_tex.bind();

  if (_need_amap_update)
  {
    if (nTextures)
    {
      if (tmp_edit_values)
      {
        std::vector<float> amap(3 * 64 * 64);
        auto& tmp_amaps = tmp_edit_values.get();

        for (int i = 0; i < 64 * 64; ++i)
        {
          for (int alpha_id = 0; alpha_id < 3; ++alpha_id)
          {
            amap[i * 3 + alpha_id] = tmp_amaps[alpha_id + 1][i] / 255.f;
          }
        }

        gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_FLOAT, amap.data());
      }
      else
      {
        std::vector<uint8_t> amap(3 * 64 * 64);
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

        gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, amap.data());
      }
    }

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
  // make sure all changes have been applied
  apply_alpha_changes();

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

namespace
{
  inline std::uint8_t float_alpha_to_uint8(float a)
  {
    return static_cast<std::uint8_t>(std::max(0.f, std::min(255.f, std::round(a))));
  }
}

bool TextureSet::apply_alpha_changes()
{
  if (!tmp_edit_values || nTextures < 2)
  {
    tmp_edit_values = boost::none;
    return false;
  }

  auto& new_amaps = tmp_edit_values.get();
  std::array<std::uint16_t, 64 * 64> totals;
  totals.fill(0);

  for (int alpha_layer = 0; alpha_layer < nTextures - 1; ++alpha_layer)
  {
    std::array<std::uint8_t, 64 * 64> values;

    for (int i = 0; i < 64 * 64; ++i)
    {
      values[i] = float_alpha_to_uint8(new_amaps[alpha_layer + 1][i]);
      totals[i] += values[i];

      // remove the possible overflow with rounding
      // max 2 if all 4 values round up so it won't change the layer's alpha much
      if (totals[i] > 255)
      {
        values[i] -= static_cast<std::uint8_t>(totals[i] - 255);
      }
    }

    alphamaps[alpha_layer]->setAlpha(values.data());
  }

  _need_amap_update = true;
  _need_lod_texture_map_update = true;

  tmp_edit_values = boost::none;

  return true;
}

void TextureSet::create_temporary_alphamaps_if_needed()
{
  if (tmp_edit_values || nTextures < 2)
  {
    return;
  }

  tmp_edit_values.emplace();

  tmp_edit_alpha_values& values = tmp_edit_values.get();

  for (int i = 0; i < 64 * 64; ++i)
  {
    float base_alpha = 255.f;

    for (int alpha_layer = 0; alpha_layer < nTextures - 1; ++alpha_layer)
    {
      float f = static_cast<float>(alphamaps[alpha_layer]->getAlpha(i));

      values[alpha_layer + 1][i] = f;
      base_alpha -= f;
    }

    values[0][i] = base_alpha;
  }
}
