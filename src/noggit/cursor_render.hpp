// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>
#include <math/vector_3d.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.fwd.hpp>

#include <memory>

namespace noggit
{
  class cursor_render
  {
  public:
    enum class mode : int
    {
      circle,
      sphere,
      square
    };

    void draw(mode cursor_mode, math::matrix_4x4 const& mvp, math::vector_4d color, math::vector_3d pos, float radius, float inner_radius_ratio = 0.f);

  private:
    bool _uploaded = false;

    void upload();

    opengl::scoped::deferred_upload_vertex_arrays<1> _vaos;
    GLuint const& _vao = _vaos[0];
    opengl::scoped::deferred_upload_buffers<1> _vbos;
    GLuint const& _vertex_vbo = _vbos[0];

    std::unique_ptr<opengl::program> _cursor_program;
  };
}
