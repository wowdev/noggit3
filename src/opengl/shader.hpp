// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/shader.fwd.hpp>
#include <opengl/types.h>

#include <initializer_list>
#include <string>
#include <set>

namespace math
{
  class matrix_4x4;
  class vector_2d;
  class vector_3d;
  class vector_4d;
}

namespace opengl
{
  struct shader
  {
    shader (GLenum type, std::string const& source);
    ~shader();

    shader (shader const&) = delete;
    shader (shader&&) = delete;
    shader& operator= (shader const&) = delete;
    shader& operator= (shader&&) = delete;

  private:
    friend struct program;

    GLuint _handle;
  };

  struct program
  {
    program (std::initializer_list<shader const*>);
    ~program();

    program (program const&) = delete;
    program (program&&) = delete;
    program& operator= (program const&) = delete;
    program& operator= (program&&) = delete;

  private:
    GLuint uniform_location (std::string const& name) const;
    GLuint attrib_location (std::string const& name) const;

    friend struct scoped::use_program;

    GLuint _handle;
  };

  namespace scoped
  {
    struct use_program
    {
      use_program (program const&);
      ~use_program();

      use_program (use_program const&) = delete;
      use_program (use_program&&) = delete;
      use_program& operator= (use_program const&) = delete;
      use_program& operator= (use_program&&) = delete;

      void uniform (std::string const& name, bool);
      void uniform (std::string const& name, math::vector_3d const&);
      void uniform (std::string const& name, math::vector_4d const&);
      void uniform (std::string const& name, math::matrix_4x4 const&);

      void sampler (std::string const& name, GLenum type, GLenum texture_slot, GLint id);

      void attrib (std::string const& name, std::vector<math::vector_2d> const&);
      void attrib (std::string const& name, math::vector_3d const*);

    private:
      program const& _program;
      std::set<GLuint> _enabled_vertex_attrib_arrays;
    };
  }
}
