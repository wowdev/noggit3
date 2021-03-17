// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/matrix_4x4.hpp>
#include <math/vector_2d.hpp>
#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <opengl/context.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>
#include <opengl/texture.hpp>

#include <boost/filesystem/string_file.hpp>

#include <QFile>
#include <QTextStream>

#include <stdexcept>
#include <list>
#include <regex>
#include <sstream>

namespace opengl
{
  shader::shader (GLenum type, std::string const& source)
  try
    : _handle (gl.createShader (type))
  {
    char const* source_ptr (source.data());
    gl.shaderSource (_handle, 1, &source_ptr, nullptr);
    gl.compile_shader (_handle);
  }
  catch (...)
  {
    std::throw_with_nested
      (std::runtime_error ("error constructing shader '" + source + "'"));
  }
  shader::~shader()
  {
    gl.deleteShader (_handle);
  }

  std::string shader::src_from_qrc(std::string const& shader_alias)
  {
    QFile f(QString::fromStdString(":/shader/" + shader_alias));

    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
      throw std::logic_error("Could not load " + shader_alias + " from the qrc file");
    }

    QTextStream stream(&f);

    return stream.readAll().toStdString();
  }

  std::string shader::src_from_qrc(std::string const& shader_alias, std::vector<std::string> const& defines)
  {
    std::string src(src_from_qrc(shader_alias));

    if (defines.empty())
    {
      return src;
    }

    std::stringstream ss;

    ss << "\n";
    for (auto const& def : defines)
    {
      ss << "#define " << def << "\n";
    }

    std::regex regex("#version[ \t]+[0-9]+.*");
    std::smatch match;

    if (!std::regex_search(src, match, regex))
    {
      throw std::logic_error("shader " + shader_alias + " has no #version directive");
    }

    // #version is always the first thing in the shader, insert defines after it
    src.insert(match.length() + match.position(), ss.str());

    return src;
  }

  program::program (std::initializer_list<shader> shaders)
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

    for (shader const& s : shaders)
    {
      attachments.emplace_back (*_handle, s._handle);
    }

    gl.link_program (*_handle);
#ifdef  VALIDATE_OPENGL_PROGRAMS
    gl.validate_program(*_handle);
#endif
  }
  program::program (program&& other)
    : _handle (boost::none)
  {
    std::swap (_handle, other._handle);
  }
  program::~program()
  {
    if (_handle)
    {
      gl.deleteProgram (*_handle);
    }
  }

  //! \todo cache lookups?
  GLuint program::uniform_location (std::string const& name) const
  {
    return gl.getUniformLocation (*_handle, name.c_str());
  }
  GLuint program::attrib_location (std::string const& name) const
  {
    return gl.getAttribLocation (*_handle, name.c_str());
  }

  namespace scoped
  {
    use_program::use_program (program const& p)
      : _program (p)
    {
      gl.getIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*> (&_old));
      gl.useProgram (*_program._handle);
    }
    use_program::~use_program()
    {
      gl.useProgram (_old);
    }

    void use_program::uniform (std::string const& name, GLint value)
    {
      gl.uniform1i (uniform_location (name), value);
    }
    void use_program::uniform (std::string const& name, GLfloat value)
    {
      gl.uniform1f (uniform_location (name), value);
    }
    void use_program::uniform (std::string const& name, std::vector<int> const& value)
    {
      gl.uniform1iv (uniform_location(name), value.size(), value.data());
    }
    void use_program::uniform (std::string const& name, math::vector_2d const& value)
    {
      gl.uniform2fv (uniform_location (name), 1, value);
    }
    void use_program::uniform (std::string const& name, math::vector_3d const& value)
    {
      gl.uniform3fv (uniform_location (name), 1, value);
    }
    void use_program::uniform (std::string const& name, math::vector_4d const& value)
    {
      gl.uniform4fv (uniform_location (name), 1, value);
    }
    void use_program::uniform (std::string const& name, math::matrix_4x4 const& value)
    {
      gl.uniformMatrix4fv (uniform_location (name), 1, GL_FALSE, value);
    }

    void use_program::sampler (std::string const& name, GLenum texture_slot, texture* tex)
    {
      uniform (name, GLint (texture_slot - GL_TEXTURE0));
      texture::set_active_texture (texture_slot - GL_TEXTURE0);
      tex->bind();
    }

    void use_program::attrib (vao_binder const&, std::string const& name, array_buffer_is_already_bound const&, math::matrix_4x4 const* data, GLuint divisor)
    {
      GLuint const location (attrib_location (name));
      math::vector_4d const* vec4_ptr = reinterpret_cast<math::vector_4d const*>(data);

      for (GLuint i = 0; i < 4; ++i)
      {
        gl.enableVertexAttribArray (location + i);
        gl.vertexAttribPointer (location + i, 4, GL_FLOAT, GL_FALSE, sizeof(math::matrix_4x4), vec4_ptr + i);
        gl.vertexAttribDivisor(location + i, divisor);
      }      
    }
    void use_program::attrib (vao_binder const&, std::string const& name, array_buffer_is_already_bound const&, GLsizei size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      gl.vertexAttribPointer (location, size, type, normalized, stride, data);
    }
    void use_program::attrib (vao_binder const&, std::string const& name, GLuint buffer, GLsizei size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      scoped::buffer_binder<GL_ARRAY_BUFFER> const bind (buffer);
      gl.vertexAttribPointer (location, size, type, normalized, stride, data);
    }

    void use_program::attrib_divisor(vao_binder const&, std::string const& name, GLuint divisor, GLsizei range)
    {
      GLuint const location (attrib_location (name));
      for (GLuint i = 0; i < range; ++i)
      {
        gl.vertexAttribDivisor(location + i, divisor);
      }
    }

    GLuint use_program::uniform_location (std::string const& name)
    {
      auto it = _uniforms.find (name);
      if (it != _uniforms.end())
      {
        return it->second;
      }

      GLuint loc = _program.uniform_location (name);
      if (loc == -1)
      {
        throw std::invalid_argument ("uniform " + name + " does not exist in shader\n");
      }
      _uniforms[name] = loc;      
      return loc;
    }

    GLuint use_program::attrib_location (std::string const& name)
    {
      auto it = _attribs.find (name);
      if (it != _attribs.end())
      {
        return it->second;
      }
      else
      {
        GLuint loc = _program.attrib_location (name);
        if (loc == -1)
        {
          throw std::invalid_argument ("attribute " + name + " does not exist in shader\n");
        }
        _attribs[name] = loc;
        return loc;
      }
    }
  }
}
