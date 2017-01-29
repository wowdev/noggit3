// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <map>
#include <string>
#include <vector>

namespace OpenGL
{
  class Texture;
}

struct scoped_blp_texture_reference;
class TextureManager
{
private:
  friend struct scoped_blp_texture_reference;
  static void delbyname(std::string name);
  static OpenGL::Texture* newTexture(std::string name);

public:
  static void report();

  //! \todo This should not be there.
  //! \note This is only for getting all cached textures in UITexturingGUI.
  static std::vector<scoped_blp_texture_reference> getAllTexturesMatching(bool(*function)(const std::string& name));

private:
  typedef std::map<std::string, OpenGL::Texture*> mapType;
  static mapType items;
};

struct scoped_blp_texture_reference
{
  scoped_blp_texture_reference (std::string const& filename)
    : _filename (filename)
    , _blp_texture (TextureManager::newTexture (_filename))
  {}

  scoped_blp_texture_reference (scoped_blp_texture_reference const& other)
    : _filename (other._filename)
    , _blp_texture (other._blp_texture ? TextureManager::newTexture (_filename) : nullptr)
  {}
  scoped_blp_texture_reference& operator= (scoped_blp_texture_reference const& other)
  {
    _filename = other._filename;
    _blp_texture = other._blp_texture ? TextureManager::newTexture (_filename) : nullptr;
    return *this;
  }

  scoped_blp_texture_reference (scoped_blp_texture_reference&& other)
    : _filename (other._filename)
    , _blp_texture (other._blp_texture)
  {
    other._blp_texture = nullptr;
  }
  scoped_blp_texture_reference& operator= (scoped_blp_texture_reference&& other)
  {
    std::swap (_filename, other._filename);
    std::swap (_blp_texture, other._blp_texture);
    other._blp_texture = nullptr;
    return *this;
  }

  ~scoped_blp_texture_reference()
  {
    if (_blp_texture)
    {
      TextureManager::delbyname (_filename);
    }
  }

  OpenGL::Texture* operator->() const
  {
    return _blp_texture;
  }

  bool operator== (scoped_blp_texture_reference const& other) const
  {
    return std::tie (_filename, _blp_texture) == std::tie (other._filename, other._blp_texture);
  }

private:
  std::string _filename;
  OpenGL::Texture* _blp_texture;
};
