// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <opengl/shader.hpp>

#include <math/matrix_4x4.h>
#include <math/vector_2d.h>
#include <math/vector_3d.h>
#include <math/vector_4d.h>

#include <opengl/context.h>
#include <opengl/texture.h>

namespace opengl
{
  shader::shader (GLenum type, std::string const& source)
    : _handle (gl.createShader (type))
  {
    char const* source_ptr (source.data());
    gl.shaderSource (_handle, 1, &source_ptr, nullptr);
    gl.compile_shader (_handle);
  }
  shader::~shader()
  {
    gl.deleteShader (_handle);
  }

  program::program (std::initializer_list<shader const*> shaders)
    : _handle (gl.createProgram())
  {
    struct scoped_attach
    {
      scoped_attach (GLuint program, GLuint shader)
        : _program (program)
        , _shader (shader)
      {
        gl.attachShader (_program, _shader);
      }
      ~scoped_attach()
      {
        gl.detachShader (_program, _shader);
      }
      GLuint _program;
      GLuint _shader;
    };

    std::list<scoped_attach> attachments;

    for (shader const* s : shaders)
    {
      attachments.emplace_back (_handle, s->_handle);
    }

    gl.link_program (_handle);
    gl.validate_program (_handle);
  }
  program::~program()
  {
    gl.deleteProgram (_handle);
  }

  //! \todo cache lookups?
  GLuint program::uniform_location (std::string const& name) const
  {
    return gl.getUniformLocation (_handle, name.c_str());
  }
  GLuint program::attrib_location (std::string const& name) const
  {
    return gl.getAttribLocation (_handle, name.c_str());
  }

  namespace scoped
  {
    use_program::use_program (program const& p)
      : _program (p)
    {
      gl.useProgram (_program._handle);
    }
    use_program::~use_program()
    {
      for (auto const& array : _enabled_vertex_attrib_arrays)
      {
        gl.disableVertexAttribArray (array);
      }
      gl.useProgram (0);
    }

    void use_program::uniform (std::string const& name, int value)
    {
      gl.uniform1i (_program.uniform_location (name), value);
    }
    void use_program::uniform (std::string const& name, float value)
    {
      gl.uniform1f (_program.uniform_location (name), value);
    }
    void use_program::uniform (std::string const& name, std::vector<int> const& value)
    {
      gl.uniform1iv(_program.uniform_location(name), value.size(), value.data());
    }
    void use_program::uniform (std::string const& name, math::vector_3d const& value)
    {
      gl.uniform3fv (_program.uniform_location (name), 1, value);
    }
    void use_program::uniform (std::string const& name, math::vector_4d const& value)
    {
      gl.uniform4fv (_program.uniform_location (name), 1, value);
    }
    void use_program::uniform (std::string const& name, math::matrix_4x4 const& value)
    {
      gl.uniformMatrix4fv (_program.uniform_location (name), 1, GL_FALSE, value);
    }

    void use_program::sampler (std::string const& name, GLenum type, GLenum texture_slot, GLint id)
    {
      uniform (name, int (texture_slot - GL_TEXTURE0));
      texture::enable_texture (texture_slot - GL_TEXTURE0);
      gl.bindTexture (GL_TEXTURE_2D, id);
    }

    void use_program::attrib (std::string const& name, std::vector<math::vector_2d> const& data)
    {
      GLuint const location (_program.attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribPointer (location, 2, GL_FLOAT, GL_FALSE, 0, data.data());
    }
    void use_program::attrib (std::string const& name, math::vector_3d const* data)
    {
      GLuint const location (_program.attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribPointer (location, 3, GL_FLOAT, GL_FALSE, 0, data);
    }
  }
}
