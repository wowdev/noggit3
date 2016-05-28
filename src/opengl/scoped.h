// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/context.h>
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
        : _was_enabled (gl.isEnabled (cap) == GL_TRUE)
      {
        if (value == GL_TRUE)
        {
          gl.enable (cap);
        }
        else
        {
          gl.disable (cap);
        }
      }

      ~bool_setter()
      {
        if (_was_enabled == GL_TRUE)
        {
          gl.enable (cap);
        }
        else
        {
          gl.disable (cap);
        }
      }

    private:
      bool _was_enabled;
    };

    template<GLboolean value>
    class depth_mask_setter
    {
    public:
      depth_mask_setter()
      {
        gl.getBooleanv (GL_DEPTH_WRITEMASK, &_was_enabled);
        gl.depthMask (value);
      }

      ~depth_mask_setter()
      {
        gl.depthMask (_was_enabled);
      }

    private:
      GLboolean _was_enabled;
    };

    template<GLenum texture_number, GLboolean value>
    class texture_setter
    {
    public:
      texture_setter()
        : _was_enabled (false)
      {
        texture::set_active_texture (texture_number);
        _was_enabled = (gl.isEnabled (GL_TEXTURE_2D) == GL_TRUE);
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
        gl.pushMatrix();
      }
      ~matrix_pusher()
      {
        gl.popMatrix();
      }
    };

    template<GLint matrix_mode>
    class matrix_mode_setter
    {
    public:
      matrix_mode_setter()
      {
        gl.getIntegerv (GL_MATRIX_MODE, &_old_mode);
        gl.matrixMode (matrix_mode);
      }
      ~matrix_mode_setter()
      {
        gl.matrixMode (_old_mode);
      }

    private:
      GLint _old_mode;
    };

    class name_pusher
    {
    public:
      name_pusher (GLuint name_id)
      {
        gl.pushName (name_id);
      }
      ~name_pusher()
      {
        gl.popName();
      }
    };
  }
}
