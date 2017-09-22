// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Brush.h>
#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <noggit/texture_set.hpp>

#include <algorithm>    // std::min
#include <iostream>     // std::cout

#include <boost/utility/in_place_factory.hpp>

void TextureSet::initTextures(MPQFile* f, MapTile* maintile, uint32_t size)
{
  // texture info
  nTextures = size / 16U;

  for (size_t i = 0; i<nTextures; ++i) 
  {
    f->read(&tex[i], 4);
    f->read(&texFlags[i], 4);
    f->read(&MCALoffset[i], 4);
    f->read(&effectID[i], 4);
    textures.emplace_back (maintile->mTextureFilenames[tex[i]]);
  }
}

void TextureSet::initAlphamaps(MPQFile* f, size_t nLayers, bool mBigAlpha, bool doNotFixAlpha)
{
  unsigned int MCALbase = f->getPos();

  for (unsigned int layer = 0; layer < nLayers; ++layer)
  {
    if (texFlags[layer] & 0x100)
    {
      f->seek(MCALbase + MCALoffset[layer]);
      alphamaps[layer - 1] = boost::in_place (f, texFlags[layer], mBigAlpha, doNotFixAlpha);
    }
  }

  // always use big alpha for editing / rendering
  if (!mBigAlpha)
  {
    convertToBigAlpha();
  }

  generate_alpha_tex();
}

int TextureSet::addTexture(scoped_blp_texture_reference texture)
{
  int texLevel = -1;

  if (nTextures < 4U)
  {
    texLevel = nTextures;
    nTextures++;

    textures.emplace_back (texture);
    texFlags[texLevel] = 0;
    effectID[texLevel] = 0;

    if (texLevel)
    {
      alphamaps[texLevel - 1] = boost::in_place();
    }
  }

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
    auto alpha_it = alphamap_tex.begin();

    if (id1)
    {
      std::swap(alphamaps[a1], alphamaps[a2]);

      for (int i = 0; i < 4096; ++i)
      {
        std::swap(*(alpha_it + a1), *(alpha_it + a2));
        alpha_it += 3;
      }
    }
    else
    {
      uint8_t alpha[4096];
      
      for (int i = 0; i < 4096; ++i)
      {
        // base_alpha = 255 - (alphamap[0] + alphamap[1] + alphamap[2])
        alpha[i] = 255 - (*alpha_it + *(alpha_it + 1) + *(alpha_it + 2));
        // update the vector with all the alphas at the same time
        *(alpha_it + a2) = alpha[i];
        alpha_it += 3;
      }

      alphamaps[a2]->setAlpha(alpha);
    }

    update_alpha_tex();
  }
}

void TextureSet::eraseTextures()
{
  for (size_t i = nTextures-1; nTextures; --i)
  {
    eraseTexture(i);
  }

  generate_alpha_tex();
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
      alphamaps[id - 1] = boost::none;
      std::swap (alphamaps[i - 1], alphamaps[i]);
    }

    textures[i] = textures[i + 1];
    texFlags[i] = texFlags[i + 1];
    effectID[i] = effectID[i + 1];
  }

  alphamaps[nTextures - 2] = boost::none;
  textures.pop_back();

  nTextures--;
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

void TextureSet::bindAlphamap(size_t id, size_t activeTexture)
{
  opengl::texture::enable_texture (activeTexture);

  alphamaps[id]->bind();
}

void TextureSet::bindTexture(size_t id, size_t activeTexture)
{
  opengl::texture::enable_texture (activeTexture);

  textures[id]->bind();
}

void TextureSet::startAnim(int id, int animtime)
{
  if (is_animated(id))
  {
    opengl::texture::set_active_texture (0);
    gl.matrixMode(GL_TEXTURE);
    gl.pushMatrix();
    
    const int spd = (texFlags[id] >> 3) & 0x7;
    const int dir = texFlags[id] & 0x7;
    const float texanimxtab[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    const float texanimytab[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];
    const int animspd = (const int)(200 * detail_size);
    float f = ((static_cast<int>(animtime*(spd / 7.0f))) % animspd) / static_cast<float>(animspd);
    gl.translatef(f*fdx, f*fdy, 0);
  }
}

void TextureSet::stopAnim(int id)
{
  if (is_animated(id))
  {
    gl.popMatrix();
    gl.matrixMode(GL_MODELVIEW);
    opengl::texture::set_active_texture (1);
  }
}

bool TextureSet::eraseUnusedTextures()
{
  if (nTextures < 2)
  {
    return false;
  }

  std::set<int> visible_tex;
  auto alpha_it (alphamap_tex.begin());

  for (int i = 0; i < 4096 && visible_tex.size() < nTextures; ++i)
  {
    uint8_t sum = 0;
    for (int n = 0; n < 3; ++n)
    {
      uint8_t a = *(alpha_it + n);
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

    alpha_it += 3;
  }

  if (visible_tex.size() < nTextures)
  {
    for (int i = 3; i >= 0; --i)
    {
      if (visible_tex.find(i) == visible_tex.end())
      {
        eraseTexture(i);
      }
    }

    generate_alpha_tex();
    return true;
  }

  return false;
}

bool TextureSet::paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
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

  //xbase, zbase mapchunk pos
  //x, y mouse pos

  int texLevel = -1;
  radius = brush->getRadius();
  dist = misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE);

  if (dist > radius)
    return changed;

  //First Lets find out do we have the texture already
  for (size_t i = 0; i<nTextures; ++i)
    if (textures[i] == texture)
      texLevel = i;

  if (texLevel == -1 && strength == 0)
  {
    return false;
  }

  if ((texLevel == -1) && (nTextures == 4) && !eraseUnusedTextures())
  {
    LogDebug << "paintTexture: No free texture slot" << std::endl;
    return false;
  }

  //Only 1 layer and its that layer
  if ((texLevel != -1) && (nTextures == 1))
    return true;

  if (texLevel == -1)
  {
    texLevel = addTexture(texture);
    if (texLevel == 0)
      return true;
    if (texLevel == -1)
    {
      LogDebug << "paintTexture: Unable to add texture." << std::endl;
      return false;
    }
  }

  zPos = zbase;
  bool texVisible[4] = { false, false, false, false };
  auto alpha_it (alphamap_tex.begin());

  for (int j = 0; j < 64; j++)
  {
    xPos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      dist = misc::dist(x, z, xPos + TEXDETAILSIZE / 2.0f, zPos + TEXDETAILSIZE / 2.0f);

      if (dist>radius)
      {
        bool baseVisible = true;
        for (size_t k = nTextures - 1; k > 0; k--)
        {
          uint8_t a = alphamaps[k - 1]->getAlpha(i + j * 64);

          if (a > 0)
          {
            texVisible[k] = true;

            if (a == 255)
            {
              baseVisible = false;
            }
          }
        }
        texVisible[0] = texVisible[0] || baseVisible;

        xPos += TEXDETAILSIZE;
        alpha_it += 3;
        continue;
      }

      float tPressure = pressure*brush->getValue(dist);
      float visibility[4] = { 255.f, 0.f , 0.f, 0.f};

      for (size_t k = 0; k < nTextures - 1; k++)
      {
        visibility[k + 1] = alphamaps[k]->getAlpha(i + j * 64);
        visibility[0] -= visibility[k + 1];
      }

      // nothing to do
      if (visibility[texLevel] == strength)
      {
        for (size_t k = 0; k < nTextures; k++)
        {
          texVisible[k] = texVisible[k] || (visibility[k] > 0.0);
        }

        xPos += TEXDETAILSIZE;
        alpha_it += 3;
        continue;
      }

      // at this point we know for sure that the textures will be changed
      changed = true;

      // alpha delta
      float diffA = (strength - visibility[texLevel])* tPressure;

      // visibility = 255 => all other at 0
      if (visibility[texLevel] + diffA >= 255.0f)
      {
        for (size_t k = 0; k < nTextures; k++)
        {
          visibility[k] = (k == texLevel) ? 255.0f : 0.0f;
        }
      }
      else
      {
        float other = 255.0f - visibility[texLevel];

        if (visibility[texLevel] == 255.0f && diffA < 0.0f)
        {
          visibility[texLevel] += diffA;
          int idTex = (!texLevel) ? 1 : texLevel - 1; // nTexture > 1 else it'd have returned true at the beginning
          visibility[idTex] -= diffA;
        }
        else
        {
          visibility[texLevel] += diffA;

          for (size_t k = 0; k < nTextures; k++)
          {
            if (k == texLevel || visibility[k] == 0)
              continue;

            visibility[k] = visibility[k] - (diffA * (visibility[k] / other));
          }
        }
      }

      for (size_t k = 0; k < nTextures; k++)
      {
        if (k < nTextures - 1)
        {
          uint8_t value = static_cast<uint8_t>(std::min(std::max(std::round(visibility[k + 1]), 0.0f), 255.0f));
          alphamaps[k]->setAlpha(i + j * 64, value);
          *(alpha_it + k) = value;
        }
        texVisible[k] = texVisible[k] || (visibility[k] > 0.0f);
      }

      alpha_it += 3;
      xPos += TEXDETAILSIZE;
    }
    zPos += TEXDETAILSIZE;
  }

  if (!changed)
  {
    return false;
  }

  bool erased = false;

  // stop after k=0 because k is unsigned
  for (size_t k = nTextures - 1; k < 4; --k)
  {  
    if (!texVisible[k])
    {
      eraseTexture(k);
      erased = true;
    }
  }

  if (erased)
  {
    generate_alpha_tex();
  }
  else
  {
    update_alpha_tex();
  }

  return changed;
}

size_t TextureSet::num()
{
  return nTextures;
}

unsigned int TextureSet::flag(size_t id)
{
  return texFlags[id];
}

unsigned int TextureSet::effect(size_t id)
{
  return effectID[id];
}

bool TextureSet::is_animated(std::size_t id) const
{
  return (id < nTextures ? (texFlags[id] & FLAG_ANIMATE) : false);
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
          texFlags[i] &= ~0x3F;
        }
        texFlags[i] |= flag;
      }
      else
      {
        texFlags[i] &= ~flag;
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

  float alphas[3] = { 0.0f, 0.0f, 0.0f };

  for (int i = 0; i < 64 * 64; ++i)
  {
    for (size_t k = 0; k < nTextures - 1; k++)
    {
      float f = static_cast<float>(*alpha(k, i));
      alphas[k] = f;
      for (size_t n = 0; n < k; n++)
        alphas[n] = (alphas[n] * ((255.0f - f)) / 255.0f);
    }

    for (size_t k = 0; k < nTextures - 1; k++)
    {
      *alpha(k, i) = static_cast<uint8_t>(std::min(std::max(std::round(alphas[k]), 0.0f), 255.0f));
    }
  }
}

void TextureSet::convertToBigAlpha()
{
  // nothing to do
  if (nTextures < 2)
    return;

  uint8_t tab[4096 * 3];

  alphas_to_big_alpha(tab);

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab + 4096 * k);
  }

  generate_alpha_tex();
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

  float alphas[3] = { 0.0f, 0.0f, 0.0f };

  for (int i = 0; i < 64 * 64; ++i)
  {
    for (size_t k = 0; k < nTextures - 1; k++)
    {
      alphas[k] = static_cast<float>(*alpha(k, i));
    }

    for (int k = nTextures - 2; k >= 0; k--)
    {
      for (int n = nTextures - 2; n > k; n--)
      {
        // prevent 0 division
        if (alphas[n] == 255.0f)
        {
          alphas[k] = 0.0f;
          break;
        }
        else
          alphas[k] = (alphas[k] / (255.0f - alphas[n])) * 255.0f;
      }
    }

    for (size_t k = 0; k < nTextures - 1; k++)
    {
      *alpha(k, i) = static_cast<uint8_t>(std::min(std::max(std::round(alphas[k]), 0.0f), 255.0f));
    }
  }
}

void TextureSet::convertToOldAlpha()
{
  // nothing to do
  if (nTextures < 2)
    return;

  uint8_t tab[3 * 4096];

  alphas_to_old_alpha(tab);

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab + k * 4096);
  }

  generate_alpha_tex();
}

void TextureSet::mergeAlpha(size_t id1, size_t id2)
{
  if (id1 >= nTextures || id2 >= nTextures || id1 == id2)
    return;

  uint8_t tab[3][64 * 64];

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    memcpy(tab[k], alphamaps[k]->getAlpha(), 64 * 64);
  }

  float alphas[3] = { 0.0f, 0.0f, 0.0f };
  float visibility[4] = { 255.0f, 0.0f, 0.0f, 0.0f };

  for (int i = 0; i < 64 * 64; ++i)
  {
    for (size_t k = 0; k < nTextures - 1; k++)
    {
      float f = static_cast<float>(tab[k][i]);
      visibility[k + 1] = f;
      for (size_t n = 0; n <= k; n++)
        visibility[n] = (visibility[n] * ((255.0f - f)) / 255.0f);
    }

    visibility[id1] += visibility[id2];
    visibility[id2] = 0;

    for (int k = nTextures - 2; k >= 0; k--)
    {
      alphas[k] = visibility[k + 1];
      for (int n = nTextures - 2; n > k; n--)
      {
        // prevent 0 division
        if (alphas[n] == 255.0f)
        {
          alphas[k] = 0.0f;
          break;
        }
        else
          alphas[k] = (alphas[k] / (255.0f - alphas[n])) * 255.0f;
      }
    }

    for (size_t k = 0; k < nTextures - 1; k++)
    {
      tab[k][i] = static_cast<uint8_t>(std::min(std::max(std::round(alphas[k]), 0.0f), 255.0f));
    }
  }

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab[k]);
  }

  eraseTexture(id2);
  generate_alpha_tex();
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
        mergeAlpha(i, j);
        changed = true;
      }
    }
  }

  return changed;
}

void TextureSet::bind_alpha(std::size_t id)
{
  opengl::texture::enable_texture (id);
  amap_gl_tex.bind();
}


void TextureSet::generate_alpha_tex()
{
  alphamap_tex.clear();

  if (nTextures < 2)
  {
    alphamap_tex.insert(alphamap_tex.end(), 4096 * 3, 0);
  }
  else
  {
    for (int i = 0; i < 4096; ++i)
    {
      for (int layer = 0; layer < 3; ++layer)
      {
        alphamap_tex.push_back(layer < nTextures - 1 ? alphamaps[layer]->getAlpha(i) : 0);
      }    
    }
  }

  update_alpha_tex();
}

void TextureSet::update_alpha_tex()
{
  amap_gl_tex.bind();
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, alphamap_tex.data());
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // for the 2D view
  if (nTextures)
  {
    for (int i = 0; i < nTextures - 1; ++i)
    {
      alphamaps[i]->loadTexture();
    }
  }
}

std::vector<uint8_t> TextureSet::lod_texture_map()
{
  std::vector<uint8_t> lod;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      std::vector<float> alpha_sum;

      for (size_t layer = 1; layer < nTextures; ++layer)
      {
        alpha_sum.push_back(0.f);
        const uint8_t* alpha = getAlpha(layer - 1);

        for (int pz = z * 8; pz < (z + 1) * 8; ++pz)
        {
          for (int px = x * 8; px < (x + 1) * 8; ++px)
          {
            alpha_sum[layer - 1] += alpha[pz * 64 + px];
          }
        }
      }

      float sum_base = 8*8*1.f - std::accumulate(alpha_sum.begin(), alpha_sum.end(), 0.f);
      alpha_sum.insert(alpha_sum.begin(), sum_base);

      int winning_layer = 0;

      for (int layer = 1; layer < nTextures; ++layer)
      {
        if (alpha_sum[winning_layer] < alpha_sum[layer])
        {
          winning_layer = layer;
        }
      }

      lod.push_back(winning_layer);
    }    
  }

  return lod;
}
