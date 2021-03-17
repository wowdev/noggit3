// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/vector_2d.hpp>
#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Log.h>
#include <opengl/context.hpp>
#include <opengl/scoped.hpp>

#include <boost/current_function.hpp>

#include <QtCore/QSysInfo>
#include <QtGui/QOpenGLFunctions>
#include <QtOpenGLExtensions/QOpenGLExtensions>

#include <functional>
#include <memory>
#include <string>
#include <vector>

opengl::context gl;

namespace opengl
{
  namespace
  {
    std::size_t inside_gl_begin_end = 0;

    template<typename Extension> struct extension_traits;
    template<> struct extension_traits<QOpenGLExtension_ARB_vertex_program>
    {
      static constexpr char const* const name = "GL_ARB_vertex_program";
    };

    struct verify_context_and_check_for_gl_errors
    {
      template<typename ExtraInfo> verify_context_and_check_for_gl_errors
          (QOpenGLContext* current_context, char const* function, ExtraInfo&& extra_info)
        : _current_context (current_context)
        , _function (function)
        , _extra_info (extra_info)
      {
        if (!_current_context)
        {
          throw std::runtime_error (std::string(_function) + ": called without active OpenGL context: no context at all");
        }
        if (!_current_context->isValid())
        {
          throw std::runtime_error (std::string(_function) + ": called without active OpenGL context: invalid");
        }
        if (QOpenGLContext::currentContext() != _current_context)
        {
          throw std::runtime_error (std::string(_function) + ": called without active OpenGL context: not current context");
        }
      }
      verify_context_and_check_for_gl_errors (QOpenGLContext* current_context, char const* function)
        : verify_context_and_check_for_gl_errors (current_context, function, &verify_context_and_check_for_gl_errors::no_extra_info)
      {}

      template<typename Functions>
        Functions* version_functions() const
      {
        Functions* f (_current_context->versionFunctions<Functions>());
        if (!f)
        {
          throw std::runtime_error (std::string(_function) + ": requires OpenGL functions for version " + typeid (Functions).name());
        }
        return f;
      }
      template<typename Extension>
        std::unique_ptr<Extension> extension_functions() const
      {
        if (!_current_context->hasExtension (extension_traits<Extension>::name))
        {
          throw std::runtime_error (std::string(_function) + ": requires OpenGL extension " + extension_traits<Extension>::name);
        }
        std::unique_ptr<Extension> functions (new Extension());
        functions->initializeOpenGLFunctions();
        return functions;
      }

      QOpenGLContext* _current_context;
      char const* _function;
      static std::string no_extra_info() { return {}; }
      std::function<std::string()> _extra_info;

      ~verify_context_and_check_for_gl_errors()
      {
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
        if (inside_gl_begin_end)
        {
          return;
        }

        std::string errors;
        std::size_t error_count = 0;
        GLenum error (GL_NO_ERROR);
        while ((error = glGetError()) != GL_NO_ERROR && error_count < 10)
        {
          switch (error)
          {
          case GL_INVALID_ENUM: errors += " GL_INVALID_ENUM"; break;
          case GL_INVALID_FRAMEBUFFER_OPERATION: errors += " GL_INVALID_FRAMEBUFFER_OPERATION"; break;
          case GL_INVALID_OPERATION: errors += " GL_INVALID_OPERATION"; break;
          case GL_INVALID_VALUE: errors += " GL_INVALID_VALUE"; break;
          case GL_OUT_OF_MEMORY: errors += " GL_OUT_OF_MEMORY"; break;
          case GL_STACK_OVERFLOW: errors += " GL_STACK_OVERFLOW"; break;
          case GL_STACK_UNDERFLOW: errors += " GL_STACK_UNDERFLOW"; break;
          case GL_TABLE_TOO_LARGE: errors += " GL_TABLE_TOO_LARGE"; break;
          default: errors += " UNKNOWN_ERROR (" + std::to_string(error) + ")"; break;
          }

          ++error_count;
        }

        if (error_count == 10 && glGetError())
        {
          errors += " and more...";
        }

        if (!errors.empty())
        {
          errors += _extra_info();
          LogError << _function << ":" + errors << std::endl;
#ifdef NOGGIT_THROW_ON_OPENGL_ERRORS
          throw std::runtime_error (std::string (_function) + ":" + errors);
#endif
        }
#endif
      }

      verify_context_and_check_for_gl_errors (verify_context_and_check_for_gl_errors const&) = delete;
      verify_context_and_check_for_gl_errors (verify_context_and_check_for_gl_errors&&) = delete;
      verify_context_and_check_for_gl_errors& operator= (verify_context_and_check_for_gl_errors const&) = delete;
      verify_context_and_check_for_gl_errors& operator= (verify_context_and_check_for_gl_errors&&) = delete;
    };
  }

  context::scoped_setter::scoped_setter (context& context_, QOpenGLContext* current_context)
    : _context (context_)
    , _old_context (_context._current_context)
    , _old_core_func (context_._3_3_core_func)
  {
    _context._current_context = current_context;
    _context._3_3_core_func = current_context->versionFunctions<QOpenGLFunctions_3_3_Core>();

    if (!_context._3_3_core_func)
    {
      throw std::runtime_error("Noggit requires OpenGL 3.3 core functions");
    }
  }
  context::scoped_setter::~scoped_setter()
  {
    _context._current_context = _old_context;
    _context._3_3_core_func = _old_core_func;
  }
  context::save_current_context::save_current_context (context& context_)
    : _is_current ( context_._current_context
                    && QOpenGLContext::currentContext() == context_._current_context
                  )
    , _gl_context (!_is_current ? nullptr : context_._current_context)
    , _surface (!_is_current ? nullptr : context_._current_context->surface())
  {
    if (_is_current)
    {
      _gl_context->doneCurrent();
    }
  }
  context::save_current_context::~save_current_context()
  {
    if (_is_current)
    {
      _gl_context->makeCurrent (_surface);
    }
  }

  void context::enable (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glEnable (target);
  }
  void context::disable (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDisable (target);
  }
  GLboolean context::isEnabled (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glIsEnabled (target);
  }
  void context::viewport (GLint x, GLint y, GLsizei width, GLsizei height)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glViewport (x, y, width, height);
  }
  void context::depthFunc (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDepthFunc (target);
  }
  void context::depthMask (GLboolean mask)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDepthMask (mask);
  }
  void context::blendFunc (GLenum sfactor, GLenum dfactor)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glBlendFunc (sfactor, dfactor);
  }

  void context::clear (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glClear (target);
  }
  void context::clearColor (GLfloat r, GLfloat g, GLfloat b, GLfloat a)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glClearColor (r, g, b, a);
  }

  void context::readBuffer (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glReadBuffer (target);
  }
  void context::readPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glReadPixels (x, y, width, height, format, type, data);
  }

  void context::lineWidth (GLfloat width)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glLineWidth (width);
  }

  void context::pointParameterf (GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glPointParameterf (pname, param);
  }
  void context::pointParameteri (GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glPointParameteri (pname, param);
  }
  void context::pointParameterfv (GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glPointParameterfv (pname, param);
  }
  void context::pointParameteriv (GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glPointParameteriv (pname, param);
  }
  void context::pointSize (GLfloat size)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glPointSize (size);
  }

  void context::hint (GLenum target, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glHint (target, mode);
  }
  void context::polygonMode (GLenum face, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glPolygonMode (face, mode);
  }

  void context::genTextures (GLuint count, GLuint* textures)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGenTextures (count, textures);
  }
  void context::deleteTextures (GLuint count, GLuint* textures)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDeleteTextures (count, textures);
  }
  void context::bindTexture (GLenum target, GLuint texture)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glBindTexture (target, texture);
  }
  void context::texImage2D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid const* data)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glTexImage2D (target, level, internal_format, width, height, border, format, type, data);
  }
  void context::compressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid const* data)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glCompressedTexImage2D (target, level, internalformat, width, height, border, imageSize, data);
  }
  void context::generateMipmap (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGenerateMipmap (target);
  }
  void context::activeTexture (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glActiveTexture (target);
  }

  void context::texParameteri (GLenum target, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glTexParameteri (target, pname, param);
  }
  void context::texParameterf (GLenum target, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glTexParameterf (target, pname, param);
  }
  void context::texParameteriv (GLenum target, GLenum pname, GLint const* params)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glTexParameteriv (target, pname, params);
  }
  void context::texParameterfv (GLenum target, GLenum pname, GLfloat const* params)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glTexParameterfv (target, pname, params);
  }

  void context::genVertexArrays (GLuint count, GLuint* arrays)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glGenVertexArrays(count, arrays);
  }
  void context::deleteVertexArray (GLuint count, GLuint* arrays)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glDeleteVertexArrays(count, arrays);
  }
  void context::bindVertexArray (GLenum array)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glBindVertexArray(array);
  }
  void context::genBuffers (GLuint count, GLuint* buffers)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGenBuffers (count, buffers);
  }
  void context::deleteBuffers (GLuint count, GLuint* buffers)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDeleteBuffers (count, buffers);
  }
  void context::bindBuffer (GLenum target, GLuint buffer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glBindBuffer (target, buffer);
  }
  void context::bufferData (GLenum target, GLsizeiptr size, GLvoid const* data, GLenum usage)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glBufferData (target, size, data, usage);
  }
  GLvoid* context::mapBuffer (GLenum target, GLenum access)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glMapBuffer (target, access);
  }
  GLboolean context::unmapBuffer (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glUnmapBuffer (target);
  }

  void context::drawElements (GLenum mode, GLsizei count, GLenum type, index_buffer_is_already_bound, std::intptr_t indices_offset)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDrawElements (mode, count, type, reinterpret_cast<void*> (indices_offset));
  }
  void context::drawElementsInstanced (GLenum mode, GLsizei count, GLsizei instancecount, GLenum type, index_buffer_is_already_bound, std::intptr_t indices_offset)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glDrawElementsInstanced (mode, count, type, reinterpret_cast<void*> (indices_offset), instancecount);
  }
  void context::drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, index_buffer_is_already_bound, std::intptr_t indices_offset)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glDrawRangeElements (mode, start, end, count, type, reinterpret_cast<void*> (indices_offset));
  }

  void context::drawElements (GLenum mode, GLsizei count, GLenum type, GLuint index_buffer, std::intptr_t indices_offset)
  {
    scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (index_buffer);
    return drawElements (mode, count, type, index_buffer_is_already_bound{}, indices_offset);
  }
  void context::drawElementsInstanced (GLenum mode, GLsizei count, GLsizei instancecount, GLenum type, GLuint index_buffer, std::intptr_t indices_offset)
  {
    scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (index_buffer);
    return drawElementsInstanced (mode, count, instancecount, type, index_buffer_is_already_bound{}, indices_offset);
  }

  namespace
  {
    template<typename> struct type_enum_for_type;
    template<> struct type_enum_for_type<unsigned char> : std::integral_constant<GLenum, GL_UNSIGNED_BYTE> {};
    template<> struct type_enum_for_type<unsigned short> : std::integral_constant<GLenum, GL_UNSIGNED_SHORT> {};
  }

  template<typename T>
    void context::drawElements (GLenum mode, GLsizei count, std::vector<T> const& indices, std::intptr_t indices_offset)
  {
#ifdef NOGGIT_OPENGL_SUPPORTS_CPU_INDICES_IN_DRAW_ELEMENTS
    return _current_context->functions()->glDrawElements (mode, count, type_enum_for_type<T>{}, reinterpret_cast<char const*> (indices.data()) + indices_offset);
#else
    scoped::buffers<1> index_buffer;
    scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (index_buffer[0]);
    bufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (T) * indices.size(), indices.data(), GL_STREAM_DRAW);
    return drawElements (mode, count, type_enum_for_type<T>{}, index_buffer_is_already_bound{}, indices_offset);
#endif
  }

  template void context::drawElements<unsigned char> (GLenum mode, GLsizei count, std::vector<unsigned char> const& indices, std::intptr_t indices_offset);
  template void context::drawElements<unsigned short> (GLenum mode, GLsizei count, std::vector<unsigned short> const& indices, std::intptr_t indices_offset);

  template<typename T>
    void context::drawElementsInstanced (GLenum mode, GLsizei count, GLsizei instancecount, std::vector<T> const& indices, std::intptr_t indices_offset)
  {
#ifdef NOGGIT_OPENGL_SUPPORTS_CPU_INDICES_IN_DRAW_ELEMENTS
    return _3_3_core_func->glDrawElementsInstanced (mode, count, type_enum_for_type<T>{}, reinterpret_cast<char const*> (indices.data()) + indices_offset, instancecount);
#else
    scoped::buffers<1> index_buffer;
    scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (index_buffer[0]);
    bufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (T) * indices.size(), indices.data(), GL_STREAM_DRAW);
    return drawElementsInstanced (mode, count, instancecount, type_enum_for_type<T>{}, index_buffer_is_already_bound{}, indices_offset);
#endif
  }

  template void context::drawElementsInstanced<unsigned short> (GLenum mode, GLsizei count, GLsizei instancecount, std::vector<unsigned short> const& indices, std::intptr_t indices_offset);

  void context::genPrograms (GLsizei count, GLuint* programs)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glGenProgramsARB (count, programs);
  }
  void context::deletePrograms (GLsizei count, GLuint* programs)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glDeleteProgramsARB (count, programs);
  }
  void context::bindProgram (GLenum target, GLuint program)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glBindProgramARB (target, program);
  }
  void context::programString (GLenum target, GLenum format, GLsizei len, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _
      ( _current_context
      , BOOST_CURRENT_FUNCTION
      , [this]
        {
          GLint error_position;
          getIntegerv (GL_PROGRAM_ERROR_POSITION_ARB, &error_position);
          return " at " + std::to_string (error_position) + ": " + reinterpret_cast<char const*> (getString (GL_PROGRAM_ERROR_STRING_ARB));
        }
      );
    return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glProgramStringARB (target, format, len, pointer);
  }
  void context::getProgramiv (GLuint program, GLenum pname, GLint* params)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGetProgramiv (program, pname, params);
  }
  void context::programLocalParameter4f (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glProgramLocalParameter4fARB (target, index, x, y, z, w);
  }

  void context::getBooleanv (GLenum target, GLboolean* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGetBooleanv (target, value);
  }
  void context::getDoublev (GLenum target, GLdouble* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glGetDoublev (target, value);
  }
  void context::getFloatv (GLenum target, GLfloat* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGetFloatv (target, value);
  }
  void context::getIntegerv (GLenum target, GLint* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGetIntegerv (target, value);
  }

  GLubyte const* context::getString (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGetString (target);
  }

  GLuint context::createShader (GLenum shader_type)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glCreateShader (shader_type);
  }
  void context::deleteShader (GLuint shader)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDeleteShader (shader);
  }
  void context::shaderSource (GLuint shader, GLsizei count, GLchar const** string, GLint const* length)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glShaderSource (shader, count, string, length);
  }
  void context::compile_shader (GLuint shader)
  {
    {
      verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
      _current_context->functions()->glCompileShader (shader);
    }
    if (get_shader (shader, GL_COMPILE_STATUS) != GL_TRUE)
    {
      std::vector<char> log (get_shader (shader, GL_INFO_LOG_LENGTH));
      _current_context->functions()->glGetShaderInfoLog (shader, log.size(), nullptr, log.data());
      LogDebug << std::string (log.data ()) << std::endl;
      throw std::runtime_error ("compiling shader failed: " + std::string (log.data()));
    }
  }
  GLint context::get_shader (GLuint shader, GLenum pname)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    GLint params;
    _current_context->functions()->glGetShaderiv (shader, pname, &params);
    return params;
  }

  GLuint context::createProgram()
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glCreateProgram();
  }
  void context::deleteProgram (GLuint program)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDeleteProgram (program);
  }
  void context::attachShader (GLuint program, GLuint shader)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glAttachShader (program, shader);
  }
  void context::detachShader (GLuint program, GLuint shader)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDetachShader (program, shader);
  }
  void context::link_program (GLuint program)
  {
    {
      verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
      _current_context->functions()->glLinkProgram (program);
    }
    if (get_program (program, GL_LINK_STATUS) != GL_TRUE)
    {
      std::string error = get_program_info_log(program);
      LogError << "linking program failed: " << error << std::endl;
      throw std::runtime_error ("linking program failed: " + error);
    }
  }
  void context::useProgram (GLuint program)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUseProgram (program);
  }
  void context::validate_program (GLuint program)
  {
    // "The issue is that Mac does not allow validating shaders before
    // they are bound to a VBO. So, validating needs to be done after
    // that, not just after compiling the shader program. The doc says
    // that this is the way to be done afaik, but Windows/Linux does
    // not seem to care."
    if (QSysInfo::productType() == "osx")
    {
      return;
    }

    {
      verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
      _current_context->functions()->glValidateProgram (program);
    }
    if (get_program (program, GL_VALIDATE_STATUS) != GL_TRUE)
    {
      std::string error = get_program_info_log(program);
      LogError << "validating program failed: " << error << std::endl;
      throw std::runtime_error("validating program failed: " + error);
    }
  }
  GLint context::get_program (GLuint program, GLenum pname)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    GLint params;
    _current_context->functions()->glGetProgramiv (program, pname, &params);
    return params;
  }
  std::string context::get_program_info_log(GLuint program)
  {
    verify_context_and_check_for_gl_errors const _(_current_context, BOOST_CURRENT_FUNCTION);
    std::vector<char> log(get_program(program, GL_INFO_LOG_LENGTH));

    if (log.empty())
    {
      return "<empty log>";
    }

    _current_context->functions()->glGetProgramInfoLog(program, log.size(), nullptr, log.data());

    return std::string(log.data());
  }

  GLint context::getAttribLocation (GLuint program, GLchar const* name)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGetAttribLocation (program, name);
  }
  void context::vertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glVertexAttribPointer (index, size, type, normalized, stride, pointer);
  }
  void context::vertexAttribDivisor (GLuint index, GLuint divisor)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _3_3_core_func->glVertexAttribDivisor(index, divisor);
  }
  void context::enableVertexAttribArray (GLuint index)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glEnableVertexAttribArray (index);
  }
  void context::disableVertexAttribArray (GLuint index)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDisableVertexAttribArray (index);
  }

  GLint context::getUniformLocation (GLuint program, GLchar const* name)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    auto val (_current_context->functions()->glGetUniformLocation (program, name));
    if (val == -1)
    {
      throw std::logic_error ("unknown uniform " + std::string (name));
    }
    return val;
  }

  void context::uniform1i (GLint location, GLint value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUniform1i (location, value);
  }
  void context::uniform1f (GLint location, GLfloat value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUniform1f (location, value);
  }

  void context::uniform1iv (GLint location, GLsizei count, GLint const* value)
  {
    verify_context_and_check_for_gl_errors const _(_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUniform1iv(location, count, value);
  }

  void context::uniform2fv (GLint location, GLsizei count, GLfloat const* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUniform2fv (location, count, value);
  }
  void context::uniform3fv (GLint location, GLsizei count, GLfloat const* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUniform3fv (location, count, value);
  }
  void context::uniform4fv (GLint location, GLsizei count, GLfloat const* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUniform4fv (location, count, value);
  }
  void context::uniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, GLfloat const* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUniformMatrix4fv (location, count, transpose, value);
  }

  void context::clearStencil (GLint s)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glClearStencil (s);
  }
  void context::stencilFunc (GLenum func, GLint ref, GLuint mask)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glStencilFunc (func, ref, mask);
  }
  void context::stencilOp (GLenum sfail, GLenum dpfail, GLenum dppass)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glStencilOp (sfail, dpfail, dppass);
  }
  void context::colorMask (GLboolean r, GLboolean g, GLboolean b, GLboolean a)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glColorMask (r, g, b, a);
  }

  void context::polygonOffset (GLfloat factor, GLfloat units)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glPolygonOffset (factor, units);
  }

  void context::genFramebuffers (GLsizei n, GLuint *ids)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGenFramebuffers (n, ids);
  }
  void context::bindFramebuffer (GLenum target, GLuint framebuffer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glBindFramebuffer (target, framebuffer);
  }
  void context::framebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glFramebufferTexture2D (target, attachment, textarget, texture, level);
  }

  void context::genRenderbuffers (GLsizei n, GLuint *ids)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGenRenderbuffers (n, ids);
  }
  void context::bindRenderbuffer (GLenum target, GLuint renderbuffer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glBindRenderbuffer (target, renderbuffer);
  }
  void context::renderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glRenderbufferStorage (target, internalformat, width, height);
  }
  void context::framebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glFramebufferRenderbuffer (target, attachment, renderbuffertarget, renderbuffer);
  }

  template<GLenum target>
    void context::bufferData (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage)
  {
    scoped::buffer_binder<target> const _ (buffer);
    return bufferData (target, size, data, usage);
  }
  template void context::bufferData<GL_ARRAY_BUFFER> (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);
  template void context::bufferData<GL_ELEMENT_ARRAY_BUFFER> (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);

  template<GLenum target, typename T>
    void context::bufferData(GLuint buffer, std::vector<T> const& data, GLenum usage)
  {
    scoped::buffer_binder<target> const _(buffer);
    return bufferData(target, sizeof(T) * data.size(), data.data(), usage);
  }

  template void context::bufferData<GL_ARRAY_BUFFER, float>(GLuint buffer, std::vector<float> const& data, GLenum usage);
  template void context::bufferData<GL_ARRAY_BUFFER, math::vector_2d>(GLuint buffer, std::vector<math::vector_2d> const& data, GLenum usage);
  template void context::bufferData<GL_ARRAY_BUFFER, math::vector_3d>(GLuint buffer, std::vector<math::vector_3d> const& data, GLenum usage);
  template void context::bufferData<GL_ARRAY_BUFFER, math::vector_4d>(GLuint buffer, std::vector<math::vector_4d> const& data, GLenum usage);
  template void context::bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint8_t>(GLuint buffer, std::vector<std::uint8_t> const& data, GLenum usage);
  template void context::bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(GLuint buffer, std::vector<std::uint16_t> const& data, GLenum usage);
  template void context::bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint32_t>(GLuint buffer, std::vector<std::uint32_t> const& data, GLenum usage);
}
