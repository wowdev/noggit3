// scoped.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __OPENGL_SCOPED_H
#define __OPENGL_SCOPED_H

#include <opengl/texture.h>

namespace opengl
{
  namespace scoped
  {
    //! \todo Ensure the if is done at compile time by template spezialization.
    template<GLenum cap, GLboolean value>
    class bool_setter
    {
    public:
      bool_setter()
        : _was_enabled (glIsEnabled (cap) == GL_TRUE)
      {
        if (value == GL_TRUE)
        {
          glEnable (cap);
        }
        else
        {
          glDisable (cap);
        }
      }

      ~bool_setter()
      {
        if (_was_enabled == GL_TRUE)
        {
          glEnable (cap);
        }
        else
        {
          glDisable (cap);
        }
      }

    private:
      bool _was_enabled;
    };

    template<GLenum texture_number, GLboolean value>
    class texture_setter
    {
    public:
      texture_setter()
        : _was_enabled (false)
      {
        texture::set_active_texture (texture_number);
        _was_enabled = (glIsEnabled (GL_TEXTURE_2D) == GL_TRUE);
        if (value == GL_TRUE)
        {
          texture::enable_texture();
        }
        else
        {
          texture::disable_texture();
        }
      }

      ~texture_setter()
      {
        texture::set_active_texture (texture_number);
        if (_was_enabled == GL_TRUE)
        {
          texture::enable_texture();
        }
        else
        {
          texture::disable_texture();
        }
      }

    private:
      bool _was_enabled;
    };

    class matrix_pusher
    {
    public:
      matrix_pusher()
      {
        glPushMatrix();
      }
      ~matrix_pusher()
      {
        glPopMatrix();
      }
    };

    template<GLint matrix_mode>
    class matrix_mode_setter
    {
    public:
      matrix_mode_setter()
      {
        glGetIntegerv (GL_MATRIX_MODE, &_old_mode);
        glMatrixMode (matrix_mode);
      }
      ~matrix_mode_setter()
      {
        glMatrixMode (_old_mode);
      }

    private:
      GLint _old_mode;
    };

    class name_pusher
    {
    public:
      name_pusher (GLuint name_id)
      {
        glPushName (name_id);
      }
      ~name_pusher()
      {
        glPopName();
      }
    };
  }
}

#endif
