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

    class vao_binder
    {
    public:
      vao_binder(GLuint vao)
      {
        gl.getIntegerv(GL_VERTEX_ARRAY_BINDING, &_old);
        gl.bindVertexArray(vao);
      }
      ~vao_binder()
      {
        gl.bindVertexArray(_old);
      }

      vao_binder (vao_binder const&) = delete;
      vao_binder (vao_binder&&) = delete;
      vao_binder& operator= (vao_binder const&) = delete;
      vao_binder& operator= (vao_binder&&) = delete;
    private:
      GLint _old;
    };

    template<GLenum type>
      struct buffer_binder
    {
      GLuint _old;
      buffer_binder (GLuint buffer)
      {
        //! \todo commented out targets not supported on macOS due to
        //! old OpenGL. If we ever need them, find workaround.
        gl.getIntegerv ( type == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING
                       //: type == GL_ATOMIC_COUNTER_BUFFER ? GL_ATOMIC_COUNTER_BUFFER_BINDING
                       //: type == GL_COPY_READ_BUFFER ? GL_COPY_READ_BUFFER_BINDING
                       //: type == GL_COPY_WRITE_BUFFER ? GL_COPY_WRITE_BUFFER_BINDING
                       : type == GL_DRAW_INDIRECT_BUFFER ? GL_DRAW_INDIRECT_BUFFER_BINDING
                       //: type == GL_DISPATCH_INDIRECT_BUFFER ? GL_DISPATCH_INDIRECT_BUFFER_BINDING
                       : type == GL_ELEMENT_ARRAY_BUFFER ? GL_ELEMENT_ARRAY_BUFFER_BINDING
                       : type == GL_PIXEL_PACK_BUFFER ? GL_PIXEL_PACK_BUFFER_BINDING
                       : type == GL_PIXEL_UNPACK_BUFFER ? GL_PIXEL_UNPACK_BUFFER_BINDING
                       //: type == GL_SHADER_STORAGE_BUFFER ? GL_SHADER_STORAGE_BUFFER_BINDING
                       : type == GL_TRANSFORM_FEEDBACK_BUFFER ? GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
                       : type == GL_UNIFORM_BUFFER ? GL_UNIFORM_BUFFER_BINDING
                       : throw std::logic_error ("bad bind target")
                       , reinterpret_cast<GLint*> (&_old)
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

    // used to bind index buffers to vao and not unbind before the vao does
    class index_buffer_manual_binder
    {
    public:
      index_buffer_manual_binder(GLuint buffer) : _buffer(buffer) {}

      void bind()
      {
        gl.getIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*> (&_old));
        gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffer);
      }

      ~index_buffer_manual_binder()
      {
        if (_binded) 
        { 
          gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _old); 
        }
      }

      index_buffer_manual_binder(index_buffer_manual_binder const&) = delete;
      index_buffer_manual_binder(index_buffer_manual_binder&&) = delete;
      index_buffer_manual_binder& operator= (index_buffer_manual_binder const&) = delete;
      index_buffer_manual_binder& operator= (index_buffer_manual_binder&&) = delete;

    private:
      GLuint _old = 0;
      GLuint _buffer;
      bool _binded = false;
    };

    template<std::size_t count>
      struct buffers
    {
      buffers()
      {
        gl.genBuffers (count, _buffers);
      }
      ~buffers()
      {
        gl.deleteBuffers (count, _buffers);
      }

      buffers (buffers const&) = delete;
      buffers (buffers&&) = delete;
      buffers& operator= (buffers const&) = delete;
      buffers& operator= (buffers&&) = delete;

      GLuint const& operator[] (std::size_t i) const { return _buffers[i]; }

    private:
      GLuint _buffers[count];
    };

    template<std::size_t count>
      struct deferred_upload_buffers
    {
      bool buffer_generated() const { return _buffer_generated; }

      void upload()
      {
        gl.genBuffers (count, _buffers);
        _buffer_generated = true;
      }

      deferred_upload_buffers() { }

      ~deferred_upload_buffers()
      {
        if (_buffer_generated)
        {
          gl.deleteBuffers (count, _buffers);
        }        
      }

      deferred_upload_buffers (deferred_upload_buffers const&) = delete;
      deferred_upload_buffers (deferred_upload_buffers&&) = delete;
      deferred_upload_buffers& operator= (deferred_upload_buffers const&) = delete;
      deferred_upload_buffers& operator= (deferred_upload_buffers&&) = delete;

      GLuint const& operator[] (std::size_t i) const { return _buffers[i]; }

    private:
      bool _buffer_generated = false;
      GLuint _buffers[count];
    };

    template<std::size_t count>
      struct deferred_upload_vertex_arrays
    {
      bool buffer_generated() const { return _buffer_generated; }

      void upload()
      {
        gl.genVertexArrays (count, _vertex_arrays);
        _buffer_generated = true;
      }

      deferred_upload_vertex_arrays() { }

      ~deferred_upload_vertex_arrays()
      {
        if (_buffer_generated)
        {
          gl.deleteVertexArray (count, _vertex_arrays);
        }        
      }

      deferred_upload_vertex_arrays (deferred_upload_vertex_arrays const&) = delete;
      deferred_upload_vertex_arrays (deferred_upload_vertex_arrays&&) = delete;
      deferred_upload_vertex_arrays& operator= (deferred_upload_vertex_arrays const&) = delete;
      deferred_upload_vertex_arrays& operator= (deferred_upload_vertex_arrays&&) = delete;

      GLuint const& operator[] (std::size_t i) const { return _vertex_arrays[i]; }

    private:
      bool _buffer_generated = false;
      GLuint _vertex_arrays[count];
    };
  }
}
