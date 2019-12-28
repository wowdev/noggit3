// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncObject.h>
#include <noggit/multimap_with_normalized_key.hpp>
#include <opengl/texture.hpp>

#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

struct BLPHeader;

struct blp_texture : public opengl::texture, AsyncObject
{
  blp_texture (std::string const& filename);
  void finishLoading();

  void loadFromUncompressedData(BLPHeader const* lHeader, char const* lData);
  void loadFromCompressedData(BLPHeader const* lHeader, char const* lData);

  int width() const { return _width; }
  int height() const { return _height; }

  void bind();
  void upload();

  virtual async_priority loading_priority() const
  {
    return async_priority::low;
  }

private:
  bool _uploaded = false;

  int _width;
  int _height;

private:
  std::map<int, std::vector<uint32_t>> _data;
  std::map<int, std::vector<uint8_t>> _compressed_data;
  boost::optional<GLint> _compression_format;
};

struct scoped_blp_texture_reference;
class TextureManager
{
public:
  static void report();

private:
  friend struct scoped_blp_texture_reference;
  static noggit::async_object_multimap_with_normalized_key<blp_texture> _;
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

  std::string get_filename()
  {
    return _filename;
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
