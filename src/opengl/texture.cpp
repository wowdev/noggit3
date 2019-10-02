// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <opengl/context.hpp>
#include <opengl/texture.hpp>

#include <utility>

namespace opengl
{
  texture::texture()
    : _id (0)
  {
    
  }

  texture::~texture()
  {
    if (_id > 0 && _id != -1)
    {
      gl.deleteTextures (1, &_id);
    }
  }

  texture::texture (texture&& other)
    : _id (other._id)
  {
    other._id = -1;
  }

  texture& texture::operator= (texture&& other)
  {
    std::swap (_id, other._id);
    return *this;
  }

  void texture::bind()
  {
    if (_id == 0)
    {
      gl.genTextures (1, &_id);
    }
    gl.bindTexture (GL_TEXTURE_2D, _id);
  }

  void texture::set_active_texture (size_t num)
  {
    gl.activeTexture (GL_TEXTURE0 + num);
  }
}
