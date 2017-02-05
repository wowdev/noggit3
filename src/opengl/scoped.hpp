// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/context.hpp>
#include <opengl/texture.hpp>

#include <stdexcept>

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

      bool_setter (bool_setter const&) = delete;
      bool_setter (bool_setter&&) = delete;
      bool_setter& operator= (bool_setter const&) = delete;
      bool_setter& operator= (bool_setter&&) = delete;

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

      depth_mask_setter (depth_mask_setter const&) = delete;
      depth_mask_setter (depth_mask_setter&&) = delete;
      depth_mask_setter& operator= (depth_mask_setter const&) = delete;
      depth_mask_setter& operator= (depth_mask_setter&&) = delete;

    private:
      GLboolean _was_enabled;
    };

    template<std::size_t texture_number, GLboolean value>
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

      texture_setter (texture_setter const&) = delete;
      texture_setter (texture_setter&&) = delete;
      texture_setter& operator= (texture_setter const&) = delete;
      texture_setter& operator= (texture_setter&&) = delete;

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

      matrix_pusher (matrix_pusher const&) = delete;
      matrix_pusher (matrix_pusher&&) = delete;
      matrix_pusher& operator= (matrix_pusher const&) = delete;
      matrix_pusher& operator= (matrix_pusher&&) = delete;
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

      matrix_mode_setter (matrix_mode_setter const&) = delete;
      matrix_mode_setter (matrix_mode_setter&&) = delete;
      matrix_mode_setter& operator= (matrix_mode_setter const&) = delete;
      matrix_mode_setter& operator= (matrix_mode_setter&&) = delete;

    private:
      GLint _old_mode;
    };

    template<GLenum type>
      struct buffer_binder
    {
      GLint _old;
      buffer_binder (GLuint buffer)
      {
        gl.getIntegerv ( type == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING
                       : type == GL_ATOMIC_COUNTER_BUFFER ? GL_ATOMIC_COUNTER_BUFFER_BINDING
                       : type == GL_COPY_READ_BUFFER ? GL_COPY_READ_BUFFER_BINDING
                       : type == GL_COPY_WRITE_BUFFER ? GL_COPY_WRITE_BUFFER_BINDING
                       : type == GL_DRAW_INDIRECT_BUFFER ? GL_DRAW_INDIRECT_BUFFER_BINDING
                       : type == GL_DISPATCH_INDIRECT_BUFFER ? GL_DISPATCH_INDIRECT_BUFFER_BINDING
                       : type == GL_ELEMENT_ARRAY_BUFFER ? GL_ELEMENT_ARRAY_BUFFER_BINDING
                       : type == GL_PIXEL_PACK_BUFFER ? GL_PIXEL_PACK_BUFFER_BINDING
                       : type == GL_PIXEL_UNPACK_BUFFER ? GL_PIXEL_UNPACK_BUFFER_BINDING
                       : type == GL_SHADER_STORAGE_BUFFER ? GL_SHADER_STORAGE_BUFFER_BINDING
                       : type == GL_TRANSFORM_FEEDBACK_BUFFER ? GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
                       : type == GL_UNIFORM_BUFFER ? GL_UNIFORM_BUFFER_BINDING
                       : throw std::logic_error ("bad bind target")
                       , &_old
                       );
        gl.bindBuffer (type, buffer);
      }
      ~buffer_binder()
      {
        gl.bindBuffer (type, _old);
      }

      buffer_binder (buffer_binder const&) = delete;
      buffer_binder (buffer_binder&&) = delete;
      buffer_binder& operator= (buffer_binder const&) = delete;
      buffer_binder& operator= (buffer_binder&&) = delete;
    };
  }
}
