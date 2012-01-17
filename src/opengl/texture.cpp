// texture.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <opengl/texture.h>

namespace opengl
{
  texture::texture()
    : _id (0)
  {
    glGenTextures (1, &_id);
  }

  texture::~texture()
  {
    glDeleteTextures (1, &_id);
    _id = 0;
  }

  void texture::bind() const
  {
    glBindTexture (GL_TEXTURE_2D, _id);
  }

  void texture::enable_texture()
  {
    glEnable (GL_TEXTURE_2D);
  }
  void texture::enable_texture (size_t num)
  {
    set_active_texture (num);
    enable_texture();
  }
  void texture::disable_texture()
  {
    glDisable (GL_TEXTURE_2D);
  }
  void texture::disable_texture (size_t num)
  {
    set_active_texture (num);
    disable_texture();
  }
  void texture::set_active_texture (size_t num)
  {
    glActiveTexture (GL_TEXTURE0 + num);
  }
}
