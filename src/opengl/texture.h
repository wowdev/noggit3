// texture.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __OPENGL_TEXTURE_H
#define __OPENGL_TEXTURE_H

#ifdef __linux__
#include <GL/glew.h>
#else
#include <gl/glew.h>
#endif

namespace opengl
{
  class texture
  {
  public:
    texture();
    ~texture();

    void bind() const;

    static void enable_texture();
    static void enable_texture (size_t num);
    static void disable_texture();
    static void disable_texture (size_t num);
    static void set_active_texture (size_t num = 0);

  protected:
    typedef GLuint internal_type;

    internal_type _id;
  };
}

#endif
