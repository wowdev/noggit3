// This file is part of Noggit3, licensed under GNU General Public License (version 3).

namespace opengl
{
  namespace scoped
  {
    template<GLenum cap>
      class bool_setter<cap, GL_TRUE>
    {
    public:
      bool_setter();
      ~bool_setter();
      bool_setter (bool_setter const&) = delete;
      bool_setter (bool_setter&&) = delete;
      bool_setter& operator= (bool_setter const&) = delete;
      bool_setter& operator= (bool_setter&&) = delete;

    private:
      bool _was_enabled;
    };

    template<GLenum cap>
      class bool_setter<cap, GL_FALSE>
    {
    public:
      bool_setter();
      ~bool_setter();
      bool_setter (bool_setter const&) = delete;
      bool_setter (bool_setter&&) = delete;
      bool_setter& operator= (bool_setter const&) = delete;
      bool_setter& operator= (bool_setter&&) = delete;

    private:
      bool _was_enabled;
    };

    template<GLenum cap>
      bool_setter<cap, GL_TRUE>::bool_setter()
        : _was_enabled (gl.isEnabled (cap) == GL_TRUE)
    {
      gl.enable (cap);
    }

    template<GLenum cap>
      bool_setter<cap, GL_TRUE>::~bool_setter()
    {
      if (_was_enabled != GL_TRUE)
      {
        gl.disable (cap);
      }
    }

    template<GLenum cap>
      bool_setter<cap, GL_FALSE>::bool_setter()
        : _was_enabled (gl.isEnabled (cap) == GL_TRUE)
    {
      gl.disable (cap);
    }

    template<GLenum cap>
      bool_setter<cap, GL_FALSE>::~bool_setter()
    {
      if (_was_enabled == GL_TRUE)
      {
        gl.enable (cap);
      }
    }

    template<GLboolean value>
      depth_mask_setter<value>::depth_mask_setter()
    {
      gl.getBooleanv (GL_DEPTH_WRITEMASK, &_was_enabled);
      gl.depthMask (value);
    }

    template<GLboolean value>
      depth_mask_setter<value>::~depth_mask_setter()
    {
      gl.depthMask (_was_enabled);
    }

    inline vao_binder::vao_binder(GLuint vao)
    {
      gl.getIntegerv(GL_VERTEX_ARRAY_BINDING, &_old);
      gl.bindVertexArray(vao);
    }
    inline vao_binder::~vao_binder()
    {
      gl.bindVertexArray(_old);
    }

    template<GLenum type>
      buffer_binder<type>::buffer_binder (GLuint buffer)
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
    template<GLenum type>
      buffer_binder<type>::~buffer_binder()
    {
      gl.bindBuffer (type, _old);
    }

    inline index_buffer_manual_binder::index_buffer_manual_binder (GLuint buffer)
      : _buffer (buffer)
    {}

    inline void index_buffer_manual_binder::bind()
    {
      _impl.emplace (_buffer);
    }

    template<std::size_t count>
      void deferred_upload_buffers<count>::upload()
    {
      gl.genBuffers (count, _buffers);
      _buffer_generated = true;
    }

    template<std::size_t count>
      deferred_upload_buffers<count>::~deferred_upload_buffers()
    {
      if (_buffer_generated)
      {
        gl.deleteBuffers (count, _buffers);
      }
    }

    template<std::size_t count>
      GLuint const& deferred_upload_buffers<count>::operator[] (std::size_t i) const
    {
      return _buffers[i];
    }

    template<std::size_t count>
      buffers<count>::buffers()
    {
      _impl.upload();
    }

    template<std::size_t count>
      GLuint const& buffers<count>::operator[] (std::size_t i) const
    {
      return _impl[i];
    }

    template<std::size_t count>
      bool deferred_upload_vertex_arrays<count>::buffer_generated() const
    {
      return _buffer_generated;
    }

    template<std::size_t count>
      void deferred_upload_vertex_arrays<count>::upload()
    {
      gl.genVertexArrays (count, _vertex_arrays);
      _buffer_generated = true;
    }

    template<std::size_t count>
      deferred_upload_vertex_arrays<count>::~deferred_upload_vertex_arrays()
    {
      if (_buffer_generated)
      {
        gl.deleteVertexArray (count, _vertex_arrays);
      }
    }

    template<std::size_t count>
      GLuint const& deferred_upload_vertex_arrays<count>::operator[] (std::size_t i) const
    {
      return _vertex_arrays[i];
    }

    template<std::size_t count>
      vertex_arrays<count>::vertex_arrays()
    {
      _impl.upload();
    }

    template<std::size_t count>
      GLuint const& vertex_arrays<count>::operator[] (std::size_t i) const
    {
      return _impl[i];
    }
  }
}
