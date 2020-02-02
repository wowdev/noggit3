// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/context.hpp>
#include <opengl/texture.hpp>

#include <boost/optional.hpp>

#include <stdexcept>

namespace opengl
{
  namespace scoped
  {
    template<GLenum cap, GLboolean value>
      class bool_setter
    {
    public:
      bool_setter();
      ~bool_setter();
      bool_setter (bool_setter const&) = delete;
      bool_setter (bool_setter&&) = delete;
      bool_setter& operator= (bool_setter const&) = delete;
      bool_setter& operator= (bool_setter&&) = delete;
    };

    template<GLboolean value>
      class depth_mask_setter
    {
    public:
      depth_mask_setter();
      ~depth_mask_setter();
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
      vao_binder(GLuint vao);
      ~vao_binder();
      vao_binder (vao_binder const&) = delete;
      vao_binder (vao_binder&&) = delete;
      vao_binder& operator= (vao_binder const&) = delete;
      vao_binder& operator= (vao_binder&&) = delete;

    private:
      GLint _old;
    };

    template<GLenum type>
      class buffer_binder
    {
    public:
      buffer_binder (GLuint buffer);
      ~buffer_binder();
      buffer_binder (buffer_binder const&) = delete;
      buffer_binder (buffer_binder&&) = delete;
      buffer_binder& operator= (buffer_binder const&) = delete;
      buffer_binder& operator= (buffer_binder&&) = delete;

    private:
      GLuint _old;
    };

    // used to bind index buffers to vao and not unbind before the vao does
    class index_buffer_manual_binder
    {
    public:
      index_buffer_manual_binder (GLuint buffer);
      ~index_buffer_manual_binder() = default;
      index_buffer_manual_binder(index_buffer_manual_binder const&) = delete;
      index_buffer_manual_binder(index_buffer_manual_binder&&) = delete;
      index_buffer_manual_binder& operator= (index_buffer_manual_binder const&) = delete;
      index_buffer_manual_binder& operator= (index_buffer_manual_binder&&) = delete;

      void bind();

    private:
      GLuint _buffer;
      boost::optional<buffer_binder<GL_ELEMENT_ARRAY_BUFFER>> _impl;
    };

    template<std::size_t count>
      class deferred_upload_buffers
    {
    public:
      deferred_upload_buffers() = default;
      ~deferred_upload_buffers();
      deferred_upload_buffers (deferred_upload_buffers const&) = delete;
      deferred_upload_buffers (deferred_upload_buffers&&) = delete;
      deferred_upload_buffers& operator= (deferred_upload_buffers const&) = delete;
      deferred_upload_buffers& operator= (deferred_upload_buffers&&) = delete;

      void upload();
      GLuint const& operator[] (std::size_t i) const;

    private:
      bool _buffer_generated = false;
      GLuint _buffers[count];
    };

    template<std::size_t count>
      class buffers
    {
    public:
      buffers();
      ~buffers() = default;
      buffers (buffers const&) = delete;
      buffers (buffers&&) = delete;
      buffers& operator= (buffers const&) = delete;
      buffers& operator= (buffers&&) = delete;

      GLuint const& operator[] (std::size_t i) const;

    private:
      deferred_upload_buffers<count> _impl;
    };

    template<std::size_t count>
      class deferred_upload_vertex_arrays
    {
    public:
      deferred_upload_vertex_arrays() = default;
      ~deferred_upload_vertex_arrays();
      deferred_upload_vertex_arrays (deferred_upload_vertex_arrays const&) = delete;
      deferred_upload_vertex_arrays (deferred_upload_vertex_arrays&&) = delete;
      deferred_upload_vertex_arrays& operator= (deferred_upload_vertex_arrays const&) = delete;
      deferred_upload_vertex_arrays& operator= (deferred_upload_vertex_arrays&&) = delete;

      void upload();
      bool buffer_generated() const;
      GLuint const& operator[] (std::size_t i) const;

    private:
      bool _buffer_generated = false;
      GLuint _vertex_arrays[count];
    };

    template<std::size_t count>
      class vertex_arrays
    {
    public:
      vertex_arrays();
      ~vertex_arrays() = default;
      vertex_arrays (vertex_arrays const&) = delete;
      vertex_arrays (vertex_arrays&&) = delete;
      vertex_arrays& operator= (vertex_arrays const&) = delete;
      vertex_arrays& operator= (vertex_arrays&&) = delete;

      GLuint const& operator[] (std::size_t i) const;

    private:
      deferred_upload_vertex_arrays<count> _impl;
    };
  }
}

#include <opengl/scoped.ipp>
