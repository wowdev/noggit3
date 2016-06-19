// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <opengl/context.h>

#include <noggit/Log.h>

#include <QOpenGLExtensions>
#include <QOpenGLFunctions_1_0>
#include <QOpenGLFunctions_1_1>
#include <QOpenGLFunctions_1_2>
#include <QOpenGLFunctions_1_3>
#include <QOpenGLFunctions_1_4>
#include <QOpenGLFunctions_1_5>

#include <boost/current_function.hpp>

#include <memory>
#include <functional>

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
          throw std::runtime_error (_function + ": called without active OpenGL context: no context at all");
        }
        if (!_current_context->isValid())
        {
          throw std::runtime_error (_function + ": called without active OpenGL context: invalid");
        }
        if (QOpenGLContext::currentContext() != _current_context)
        {
          throw std::runtime_error (_function + ": called without active OpenGL context: not current context");
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
          throw std::runtime_error (_function + ": requires OpenGL functions for version " + typeid (Functions).name());
        }
        return f;
      }
      template<typename Extension>
        std::unique_ptr<Extension> extension_functions() const
      {
        if (!_current_context->hasExtension (extension_traits<Extension>::name))
        {
          throw std::runtime_error (_function + ": requires OpenGL extension " + extension_traits<Extension>::name);
        }
        std::unique_ptr<Extension> functions (new Extension());
        functions->initializeOpenGLFunctions();
        return functions;
      }

      QOpenGLContext* _current_context;
      std::string _function;
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
        while (GLenum error = _current_context->functions()->glGetError())
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
          default: errors += " UNKNOWN_ERROR"; break;
          }
        }
        if (!errors.empty())
        {
          errors += _extra_info();
#ifndef NOGGIT_DO_NOT_THROW_ON_OPENGL_ERRORS
          LogError << _function + ":" + errors << "\n";
#else
          throw std::runtime_error (_function + ":" + errors);
#endif
        }
#endif
      }
    };
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

  void context::begin (GLenum target)
  {
    ++inside_gl_begin_end;
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glBegin (target);
  }
  void context::end()
  {
    --inside_gl_begin_end;
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glEnd();
  }

  void context::enableClientState (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_1>()->glEnableClientState (target);
  }
  void context::disableClientState (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_1>()->glDisableClientState (target);
  }
  void context::clientActiveTexture (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_3>()->glClientActiveTexture (target);
  }

  void context::normal3f (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glNormal3f (x, y, z);
  }
  void context::normal3fv (GLfloat const data[3])
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glNormal3fv (data);
  }
  void context::vertex2f (GLfloat x, GLfloat y)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glVertex2f (x, y);
  }
  void context::vertex3f (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glVertex3f (x, y, z);
  }
  void context::vertex3fv (GLfloat const data[3])
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glVertex3fv (data);
  }
  void context::color3f (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glColor3f (x, y, z);
  }
  void context::color4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glColor4f (x, y, z, w);
  }
  void context::color4ub (GLubyte x, GLubyte y, GLubyte z, GLubyte w)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glColor4ub (x, y, z, w);
  }
  void context::color3fv (GLfloat const data[3])
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glColor3fv (data);
  }
  void context::color4fv (GLfloat const data[4])
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glColor4fv (data);
  }
  void context::texCoord2f (GLfloat x, GLfloat y)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexCoord2f (x, y);
  }
  void context::texCoord2fv (GLfloat const data[2])
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexCoord2fv (data);
  }
  void context::multiTexCoord2f (GLenum target, GLfloat x, GLfloat y)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_3>()->glMultiTexCoord2f (target, x, y);
  }

  void context::matrixMode (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glMatrixMode (target);
  }
  void context::pushMatrix()
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glPushMatrix();
  }
  void context::popMatrix()
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glPopMatrix();
  }
  void context::loadIdentity()
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLoadIdentity();
  }
  void context::translatef (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTranslatef (x, y, z);
  }
  void context::scalef (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glScalef (x, y, z);
  }
  void context::rotatef (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glRotatef (x, y, z, w);
  }
  void context::multMatrixf (GLfloat const data[4])
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glMultMatrixf (data);
  }

  void context::ortho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glOrtho (left, right, bottom, top, nearVal, farVal);
  }
  void context::frustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glFrustum (left, right, bottom, top, nearVal, farVal);
  }
  void context::viewport (GLint x, GLint y, GLsizei width, GLsizei height)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glViewport (x, y, width, height);
  }

  void context::alphaFunc (GLenum func, GLfloat ref)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glAlphaFunc (func, ref);
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
  void context::shadeModel (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glShadeModel (target);
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
    return _.version_functions<QOpenGLFunctions_1_0>()->glReadBuffer (target);
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
    return _.version_functions<QOpenGLFunctions_1_4>()->glPointParameterf (pname, param);
  }
  void context::pointParameteri (GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_4>()->glPointParameteri (pname, param);
  }
  void context::pointParameterfv (GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_4>()->glPointParameterfv (pname, param);
  }
  void context::pointParameteriv (GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_4>()->glPointParameteriv (pname, param);
  }
  void context::pointSize (GLfloat size)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glLineWidth (size);
  }

  void context::hint (GLenum target, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glHint (target, mode);
  }
  void context::polygonMode (GLenum face, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glPolygonMode (face, mode);
  }
  GLint context::renderMode (GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glRenderMode (mode);
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

  void context::texEnvf (GLenum target, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexEnvf (target, pname, param);
  }
  void context::texEnvi (GLenum target, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexEnvi (target, pname, param);
  }

  void context::texGeni (GLenum coord, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexGeni (coord, pname, param);
  }
  void context::texGenf (GLenum coord, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexGenf (coord, pname, param);
  }
  void context::texGend (GLenum coord, GLenum pname, GLdouble param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexGend (coord, pname, param);
  }
  void context::texGeniv (GLenum coord, GLenum pname, GLint const* params)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexGeniv (coord, pname, params);
  }
  void context::texGenfv (GLenum coord, GLenum pname, GLfloat const* params)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexGenfv (coord, pname, params);
  }
  void context::texGendv (GLenum coord, GLenum pname, GLdouble const* params)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glTexGendv (coord, pname, params);
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
    return _.version_functions<QOpenGLFunctions_1_5>()->glMapBuffer (target, access);
  }
  GLboolean context::unmapBuffer (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_5>()->glUnmapBuffer (target);
  }
  void context::drawElements (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glDrawElements (mode, count, type, indices);
  }
  void context::drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid const* indices)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_2>()->glDrawRangeElements (mode, start, end, count, type, indices);
  }

  void context::vertexPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_1>()->glVertexPointer (size, type, stride, pointer);
  }
  void context::colorPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_1>()->glColorPointer (size, type, stride, pointer);
  }
  void context::texCoordPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_1>()->glTexCoordPointer (size, type, stride, pointer);
  }
  void context::normalPointer (GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_1>()->glNormalPointer (type, stride, pointer);
  }

  GLuint context::genLists (GLsizei range)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glGenLists (range);
  }
  void context::deleteLists (GLuint list, GLsizei range)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glDeleteLists (list, range);
  }
  void context::newList (GLuint list, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glNewList (list, mode);
  }
  void context::endList()
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glEndList();
  }
  void context::callList (GLuint list)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glCallList (list);
  }

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

  void context::lightf (GLenum light, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLightf (light, pname, param);
  }
  void context::lighti (GLenum light, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLighti (light, pname, param);
  }
  void context::lightfv (GLenum light, GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLightfv (light, pname, param);
  }
  void context::lightiv (GLenum light, GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLightiv (light, pname, param);
  }
  void context::lightModelf (GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLightModelf (pname, param);
  }
  void context::lightModeli (GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLightModeli (pname, param);
  }
  void context::lightModelfv (GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLightModelfv (pname, param);
  }
  void context::lightModeliv (GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glLightModeliv (pname, param);
  }

  void context::materiali (GLenum face, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glMateriali (face, pname, param);
  }
  void context::materialf (GLenum face, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glMaterialf (face, pname, param);
  }
  void context::materialiv (GLenum face, GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glMaterialiv (face, pname, param);
  }
  void context::materialfv (GLenum face, GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glMaterialfv (face, pname, param);
  }
  void context::colorMaterial (GLenum face, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glColorMaterial (face, mode);
  }

  void context::fogi (GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glFogi (pname, param);
  }
  void context::fogiv (GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glFogiv (pname, param);
  }
  void context::fogf (GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glFogf (pname, param);
  }
  void context::fogfv (GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glFogfv (pname, param);
  }

  void context::getBooleanv (GLenum target, GLboolean* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glGetBooleanv (target, value);
  }
  void context::getDoublev (GLenum target, GLdouble* value)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _.version_functions<QOpenGLFunctions_1_0>()->glGetDoublev (target, value);
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
      std::vector<char> log (get_program (program, GL_INFO_LOG_LENGTH));
      _current_context->functions()->glGetProgramInfoLog (program, log.size(), nullptr, log.data());
      throw std::runtime_error ("linking program failed: " + std::string (log.data()));
    }
  }
  void context::useProgram (GLuint program)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    return _current_context->functions()->glUseProgram (program);
  }
  void context::validate_program (GLuint program)
  {
    {
      verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
      _current_context->functions()->glValidateProgram (program);
    }
    if (get_program (program, GL_VALIDATE_STATUS) != GL_TRUE)
    {
      //! \todo show log
      throw std::runtime_error ("validating program failed");
    }
  }
  GLint context::get_program (GLuint program, GLenum pname)
  {
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
    GLint params;
    _current_context->functions()->glGetProgramiv (program, pname, &params);
    return params;
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
}
