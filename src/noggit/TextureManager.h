// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/multimap_with_normalized_key.hpp>
#include <opengl/texture.hpp>

#include <map>
#include <string>
#include <vector>

struct BLPHeader;

struct blp_texture : public opengl::texture
{
  blp_texture (std::string const& filename);

  void loadFromUncompressedData(BLPHeader const* lHeader, char const* lData);
  void loadFromCompressedData(BLPHeader const* lHeader, char const* lData);

  const std::string& filename();
  int width() const { return original_width; }
  int height() const { return original_height; }

private:
  int original_width;
  int original_height;
  int _width;
  int _height;
  std::string _filename;
};

struct scoped_blp_texture_reference;
class TextureManager
{
public:
  static void report();

  //! \todo This should not be there.
  //! \note This is only for getting all cached textures in UITexturingGUI.
  static std::vector<scoped_blp_texture_reference> getAllTexturesMatching(bool(*function)(const std::string& name));

private:
  friend struct scoped_blp_texture_reference;
  static noggit::multimap_with_normalized_key<blp_texture> _;
};

struct scoped_blp_texture_reference
{
  scoped_blp_texture_reference (std::string const& filename)
    : _filename (filename)
    , _blp_texture (TextureManager::_.emplace (_filename))
  {}

  scoped_blp_texture_reference (scoped_blp_texture_reference const& other)
    : _filename (other._filename)
    , _blp_texture (other._blp_texture ? TextureManager::_.emplace (_filename) : nullptr)
  {}
  scoped_blp_texture_reference& operator= (scoped_blp_texture_reference const& other)
  {
    _filename = other._filename;
    _blp_texture = other._blp_texture ? TextureManager::_.emplace (_filename) : nullptr;
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
      TextureManager::_.erase (_filename);
    }
  }

  blp_texture* operator->() const
  {
    return _blp_texture;
  }
  blp_texture* get() const
  {
    return _blp_texture;
  }

  bool operator== (scoped_blp_texture_reference const& other) const
  {
    return std::tie (_filename, _blp_texture) == std::tie (other._filename, other._blp_texture);
  }

private:
  std::string _filename;
  blp_texture* _blp_texture;
};

namespace noggit
{
  QPixmap render_blp_to_pixmap ( std::string const& blp_filename
                               , int width = -1
                               , int height = -1
                               );
}
