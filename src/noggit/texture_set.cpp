#include <noggit/texture_set.hpp>

#include <iostream>     // std::cout
#include <algorithm>    // std::min

#include <noggit/MapHeaders.h>
#include <noggit/MapTile.h>
#include <noggit/Log.h>
#include <noggit/World.h>
#include <noggit/Brush.h>

#include <opengl/context.h>

namespace noggit
{
  texture_set::texture_set()
  {}

  texture_set::~texture_set()
  {
    for (size_t i = 1; i < _textures.size(); ++i)
      delete _alphamaps[i - 1];
  }

  void texture_set::initTextures(mpq::file* f, texture_names_type* texture_names, uint32_t size)
  {
    // texture info

    for (size_t i = 0; i < size / 16U; ++i) {
      f->read(&_tex[i], 4);
      f->read(&_tex_flags[i], 4);
      f->read(&_mcal_offset[i], 4);
      f->read(&_effect_id[i], 4);

      if (_tex_flags[i] & FLAG_ANIMATE)
      {
        _animated[i] = _tex_flags[i];
      }
      else
      {
        _animated[i] = 0;
      }
      _textures.emplace_back(texture_names->at(_tex[i]));
    }
  }

  void texture_set::initAlphamaps(mpq::file* f, bool mBigAlpha, bool doNotFixAlpha)
  {
    unsigned int MCALbase = f->getPos();

    for (size_t i = 0; i < 3; ++i)
    {
      _alphamaps[i] = NULL;
    }

    for (unsigned int layer = 0; layer < _textures.size(); ++layer)
    {
      if (_tex_flags[layer] & 0x100)
      {
        f->seek(MCALbase + _mcal_offset[layer]);
        _alphamaps[layer - 1] = new alphamap(f, _tex_flags[layer], mBigAlpha, doNotFixAlpha);
      }
    }
  }

  int texture_set::addTexture(scoped_blp_texture_reference &texture)
  {
    int texLevel = -1;

    if (_textures.size() < 4U)
    {
      texLevel = _textures.size();

      _textures.push_back(texture);
      _animated[texLevel] = 0;
      _tex_flags[texLevel] = 0;
      _effect_id[texLevel] = 0;

      if (texLevel)
      {
        if (_alphamaps[texLevel - 1])
        {
          LogError << "Alpha Map has invalid texture binding" << std::endl;
          _textures.pop_back();
          return -1;
        }
        _alphamaps[texLevel - 1] = new alphamap();
      }
    }

    return texLevel;
  }

  void texture_set::switchTexture(scoped_blp_texture_reference &oldTexture, scoped_blp_texture_reference &newTexture)
  {
    int texLevel = -1;
    for (size_t i = 0; i < _textures.size(); ++i)
    {
      if (_textures[i] == oldTexture)
        texLevel = i;
      // prevent texture duplication
      if (_textures[i] == newTexture)
        return;
    }

    if (texLevel != -1)
    {
      _textures[texLevel] = newTexture;
    }
  }

  void texture_set::eraseTextures()
  {
    for (size_t i = 0; i < _textures.size(); ++i)
    {
      _tex[i] = 0;

      if (i < 1) continue;

      delete _alphamaps[i - 1];
      _alphamaps[i - 1] = nullptr;
    }

    _textures.clear();
  }

  const std::string texture_set::filename(size_t id)
  {
    return _textures[id]->filename().toStdString();
  }

  void texture_set::bindAlphamap(size_t id, size_t activeTexture) const
  {
    opengl::texture::enable_texture(activeTexture);

    _alphamaps[id]->bind();
  }

  void texture_set::bindTexture(size_t id, size_t activeTexture) const
  {
    opengl::texture::enable_texture(activeTexture);

    _textures[id]->bind();
  }

  bool texture_set::paintTexture(float xbase, float zbase, float x, float z, const brush& brush, float strength, float pressure, noggit::scoped_blp_texture_reference texture)
  {
#if 1
    float zPos, xPos, change, xdiff, zdiff, dist, radius;

    int texLevel = -1;

    radius = brush.getRadius();

    xdiff = xbase - x + CHUNKSIZE / 2;
    zdiff = zbase - z + CHUNKSIZE / 2;
    dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);

    if (dist > (radius + MAPCHUNK_RADIUS))
      return false;

    //First Lets find out do we have the texture already
    for (size_t i = 0; i<_textures.size(); ++i)
      if (_textures[i] == texture)
        texLevel = i;


    if ((texLevel == -1) && (_textures.size() == 4))
    {
      // Implement here auto texture slot freeing :)
      LogDebug << "paintTexture: No free texture slot" << std::endl;
      return false;
    }

    //Only 1 layer and its that layer
    if ((texLevel != -1) && (_textures.size() == 1))
      return true;


    change = CHUNKSIZE / 62.0f;
    zPos = zbase;

    float target, tarAbove, tPressure;
    //int texAbove=_textures.size()-texLevel-1;


    for (int j = 0; j < 63; j++)
    {
      xPos = xbase;
      for (int i = 0; i < 63; ++i)
      {
        xdiff = xPos - x;
        zdiff = zPos - z;
        dist = std::abs(std::sqrt(xdiff*xdiff + zdiff*zdiff));

        if (dist>radius)
        {
          xPos += change;
          continue;
        }

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

        target = strength;
        tarAbove = 1 - target;

        tPressure = pressure*brush.getValue(dist);

        if (texLevel > 0)
        {
          uchar value = static_cast<unsigned char>(std::max(std::min((1.0f - tPressure) * (static_cast<float>(_alphamaps[texLevel - 1]->getAlpha(i + j * 64))) + tPressure * target + 0.5f, 255.0f), 0.0f));
          _alphamaps[texLevel - 1]->setAlpha(i + j * 64, value);
        }

        for (size_t k = texLevel; k<_textures.size() - 1; k++)
        {
          uchar value = static_cast<unsigned char>(std::max(std::min((1.0f - tPressure) * (static_cast<float>(_alphamaps[k]->getAlpha(i + j * 64))) + tPressure * target + 0.5f, 255.0f), 0.0f));
          _alphamaps[k]->setAlpha(i + j * 64, value);
        }

        xPos += change;
      }
      zPos += change;
    }

    if (texLevel == -1)
    {
      LogDebug << "Somehow no texture got painted." << std::endl;
      return false;
    }

    for (size_t j = texLevel; j < _textures.size() - 1; j++)
    {
      if (j > 2)
      {
        LogError << "WTF how did you get here??? Get a cookie." << std::endl;
        continue;
      }
      _alphamaps[j]->loadTexture();
    }

    if (texLevel)
    {
      _alphamaps[texLevel - 1]->loadTexture();
    }
#else
    // new stuff from bernd.
    // need to get rework. Add old code with switch that the guys out there can use paint.
    const float radius = Brush.getRadius();

    // Are we really painting on this chunk?
    const float xdiff = xbase + CHUNKSIZE / 2 - x;
    const float zdiff = zbase + CHUNKSIZE / 2 - z;

    if ((xdiff * xdiff + zdiff * zdiff) > (MAPCHUNK_RADIUS + radius) * (MAPCHUNK_RADIUS + radius))
      return false;


    // Search for empty layer.
    int texLevel = -1;

    for (size_t i = 0; i < _textures.size(); ++i)
    {
      if (_textures[i] == texture)
      {
        texLevel = i;
      }
    }

    if (texLevel == -1)
    {

      if (_textures.size() == 4)
      {
        for (size_t layer = 0; layer < _textures.size(); ++layer)
        {
          unsigned char map[64 * 64];
          if (layer)
            memcpy(map, amap[layer - 1], 64 * 64);
          else
            memset(map, 255, 64 * 64);

          for (size_t layerAbove = layer + 1; layerAbove < _textures.size(); ++layerAbove)
          {
            unsigned char* above = amap[layerAbove - 1];
            for (size_t i = 0; i < 64 * 64; ++i)
            {
              map[i] = std::max(0, map[i] - above[i]);
            }
          }

          size_t sum = 0;
          for (size_t i = 0; i < 64 * 64; ++i)
          {
            sum += map[i];
          }

          if (!sum)
          {
            for (size_t i = layer; i < _textures.size() - 1; ++i)
            {
              _textures[i] = _textures[i + 1];
              animated[i] = animated[i + 1];
              texture_flags(i, texture_flags(i + 1));
              texture_effect_id(i, texture_effect_id(i + 1));
              if (i)
                memcpy(amap[i - 1], amap[i], 64 * 64);
            }

            for (size_t j = layer; j < _textures.size(); j++)
            {
              gl.bindTexture(GL_TEXTURE_2D, _alphamaps[j - 1]);
              gl.texImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1]);
            }
            _textures.pop_back();
          }
        }
      }

      if (_textures.size() == 4)
        return false;

      texLevel = addTexture(texture);

    }
    else
    {
      if (_textures.size() == 1)
        return true;
    }
    LogDebug << "TexLevel: " << texLevel << " -  _textures.size(): " << _textures.size() << "\n";
    // We now have a texture at texLevel > 0.
    static const float change = CHUNKSIZE / 62.0f; //! \todo 64? 63? 62? Wtf?

    if (texLevel == 0)
      return true;

    for (size_t j = 0; j < 64; ++j)
    {
      for (size_t i = 0; i < 64; ++i)
      {
        const float xdiff_ = xbase + change * i - x;
        const float zdiff_ = zbase + change * j - z;
        const float dist = std::sqrt(xdiff_ * xdiff_ + zdiff_ * zdiff_);

        if (dist <= radius)
        {
          amap[texLevel - 1][i + j * 64] = (unsigned char)(std::max(std::min(amap[texLevel - 1][i + j * 64] + pressure * strength * Brush.getValue(dist) + 0.5f, 255.0f), 0.0f));
        }
      }
    }


    // Redraw changed layers.

    for (size_t j = texLevel; j < _textures.size(); j++)
    {
      gl.bindTexture(GL_TEXTURE_2D, _alphamaps[j - 1]);
      gl.texImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1]);
    }
#endif

    return true;
  }

  const size_t texture_set::num() const
  {
    return _textures.size();
  }

  const int texture_set::animated(size_t id) const
  {
    return _animated[id];
  }

  const unsigned int texture_set::flag(size_t id) const
  {
    return _tex_flags[id];
  }

  const unsigned int texture_set::effect(size_t id) const
  {
    return _effect_id[id];
  }

  void texture_set::setAlpha(size_t id, size_t offset, unsigned char value)
  {
    _alphamaps[id]->setAlpha(offset, value);
  }

  void texture_set::setAlpha(size_t id, unsigned char *amap)
  {
    _alphamaps[id]->setAlpha(amap);
  }

  const unsigned char texture_set::getAlpha(size_t id, size_t offset)
  {
    return _alphamaps[id]->getAlpha(offset);
  }

  const unsigned char *texture_set::getAlpha(size_t id)
  {
    return _alphamaps[id]->getAlpha();
  }

  scoped_blp_texture_reference texture_set::texture(size_t id)
  {
    return _textures[id];
  }
}
