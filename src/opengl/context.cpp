// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Log.h>
#include <opengl/context.hpp>
#include <opengl/scoped.hpp>

#include <boost/current_function.hpp>

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

    struct verify_context_and_check_for_gl_errors
    {
      template<typename ExtraInfo> verify_context_and_check_for_gl_errors
          (char const* function, ExtraInfo&& extra_info)
        : _function (function)
        , _extra_info (extra_info)
      {}
      verify_context_and_check_for_gl_errors (char const* function)
        : verify_context_and_check_for_gl_errors (function, &verify_context_and_check_for_gl_errors::no_extra_info)
      {}

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
        while (GLenum error = glGetError())
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
#ifndef NOGGIT_THROW_ON_OPENGL_ERRORS
          LogError << _function + ":" + errors << "\n";
#else
          throw std::runtime_error (_function + ":" + errors);
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

  void context::enable (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glEnable (target);
  }
  void context::disable (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDisable (target);
  }
  GLboolean context::isEnabled (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glIsEnabled (target);
  }

  void context::begin (GLenum target)
  {
    ++inside_gl_begin_end;
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBegin (target);
  }
  void context::end()
  {
    --inside_gl_begin_end;
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glEnd();
  }

  void context::enableClientState (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glEnableClientState (target);
  }
  void context::disableClientState (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDisableClientState (target);
  }
  void context::clientActiveTexture (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glClientActiveTexture (target);
  }

  void context::normal3f (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glNormal3f (x, y, z);
  }
  void context::normal3fv (GLfloat const data[3])
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glNormal3fv (data);
  }
  void context::vertex2f (GLfloat x, GLfloat y)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glVertex2f (x, y);
  }
  void context::vertex3f (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glVertex3f (x, y, z);
  }
  void context::vertex3fv (GLfloat const data[3])
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glVertex3fv (data);
  }
  void context::color3f (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColor3f (x, y, z);
  }
  void context::color4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColor4f (x, y, z, w);
  }
  void context::color4ub (GLubyte x, GLubyte y, GLubyte z, GLubyte w)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColor4ub (x, y, z, w);
  }
  void context::color3fv (GLfloat const data[3])
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColor3fv (data);
  }
  void context::color4fv (GLfloat const data[4])
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColor4fv (data);
  }
  void context::texCoord2f (GLfloat x, GLfloat y)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexCoord2f (x, y);
  }
  void context::texCoord2fv (GLfloat const data[2])
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexCoord2fv (data);
  }
  void context::multiTexCoord2f (GLenum target, GLfloat x, GLfloat y)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMultiTexCoord2f (target, x, y);
  }

  void context::matrixMode (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMatrixMode (target);
  }
  void context::pushMatrix()
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPushMatrix();
  }
  void context::popMatrix()
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPopMatrix();
  }
  void context::loadIdentity()
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLoadIdentity();
  }
  void context::translatef (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTranslatef (x, y, z);
  }
  void context::scalef (GLfloat x, GLfloat y, GLfloat z)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glScalef (x, y, z);
  }
  void context::rotatef (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glRotatef (x, y, z, w);
  }
  void context::multMatrixf (GLfloat const* data)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMultMatrixf (data);
  }

  void context::ortho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glOrtho (left, right, bottom, top, nearVal, farVal);
  }
  void context::frustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glFrustum (left, right, bottom, top, nearVal, farVal);
  }
  void context::viewport (GLint x, GLint y, GLsizei width, GLsizei height)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glViewport (x, y, width, height);
  }

  void context::alphaFunc (GLenum func, GLfloat ref)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glAlphaFunc (func, ref);
  }
  void context::depthFunc (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDepthFunc (target);
  }
  void context::depthMask (GLboolean mask)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDepthMask (mask);
  }
  void context::blendFunc (GLenum sfactor, GLenum dfactor)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBlendFunc (sfactor, dfactor);
  }
  void context::shadeModel (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glShadeModel (target);
  }

  void context::clear (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glClear (target);
  }
  void context::clearColor (GLfloat r, GLfloat g, GLfloat b, GLfloat a)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glClearColor (r, g, b, a);
  }

  void context::readBuffer (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glReadBuffer (target);
  }
  void context::readPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glReadPixels (x, y, width, height, format, type, data);
  }

  void context::lineWidth (GLfloat width)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLineWidth (width);
  }
  void context::lineStipple (GLint factor, GLushort pattern)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLineStipple (factor, pattern);
  }

  void context::pointParameterf (GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPointParameterf (pname, param);
  }
  void context::pointParameteri (GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPointParameteri (pname, param);
  }
  void context::pointParameterfv (GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPointParameterfv (pname, param);
  }
  void context::pointParameteriv (GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPointParameteriv (pname, param);
  }
  void context::pointSize (GLfloat size)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPointSize (size);
  }

  void context::hint (GLenum target, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glHint (target, mode);
  }
  void context::polygonMode (GLenum face, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPolygonMode (face, mode);
  }
  GLint context::renderMode (GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glRenderMode (mode);
  }

  void context::genTextures (GLuint count, GLuint* textures)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGenTextures (count, textures);
  }
  void context::deleteTextures (GLuint count, GLuint* textures)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDeleteTextures (count, textures);
  }
  void context::bindTexture (GLenum target, GLuint texture)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBindTexture (target, texture);
  }
  void context::texImage2D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid const* data)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexImage2D (target, level, internal_format, width, height, border, format, type, data);
  }
  void context::compressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid const* data)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glCompressedTexImage2D (target, level, internalformat, width, height, border, imageSize, data);
  }
  void context::generateMipmap (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGenerateMipmap (target);
  }
  void context::activeTexture (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glActiveTexture (target);
  }

  void context::texEnvf (GLenum target, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexEnvf (target, pname, param);
  }
  void context::texEnvi (GLenum target, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexEnvi (target, pname, param);
  }

  void context::texGeni (GLenum coord, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexGeni (coord, pname, param);
  }
  void context::texGenf (GLenum coord, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexGenf (coord, pname, param);
  }
  void context::texGend (GLenum coord, GLenum pname, GLdouble param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexGend (coord, pname, param);
  }
  void context::texGeniv (GLenum coord, GLenum pname, GLint const* params)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexGeniv (coord, pname, params);
  }
  void context::texGenfv (GLenum coord, GLenum pname, GLfloat const* params)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexGenfv (coord, pname, params);
  }
  void context::texGendv (GLenum coord, GLenum pname, GLdouble const* params)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexGendv (coord, pname, params);
  }

  void context::texParameteri (GLenum target, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexParameteri (target, pname, param);
  }
  void context::texParameterf (GLenum target, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexParameterf (target, pname, param);
  }
  void context::texParameteriv (GLenum target, GLenum pname, GLint const* params)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexParameteriv (target, pname, params);
  }
  void context::texParameterfv (GLenum target, GLenum pname, GLfloat const* params)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexParameterfv (target, pname, params);
  }

  void context::genBuffers (GLuint count, GLuint* buffers)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGenBuffers (count, buffers);
  }
  void context::deleteBuffers (GLuint count, GLuint* buffers)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDeleteBuffers (count, buffers);
  }
  void context::bindBuffer (GLenum target, GLuint buffer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBindBuffer (target, buffer);
  }
  void context::bufferData (GLenum target, GLsizeiptr size, GLvoid const* data, GLenum usage)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBufferData (target, size, data, usage);
  }
  GLvoid* context::mapBuffer (GLenum target, GLenum access)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMapBuffer (target, access);
  }
  GLboolean context::unmapBuffer (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUnmapBuffer (target);
  }
  void context::drawElements (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDrawElements (mode, count, type, indices);
  }
  void context::drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid const* indices)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDrawRangeElements (mode, start, end, count, type, indices);
  }

  void context::vertexPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glVertexPointer (size, type, stride, pointer);
  }
  void context::colorPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColorPointer (size, type, stride, pointer);
  }
  void context::texCoordPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glTexCoordPointer (size, type, stride, pointer);
  }
  void context::normalPointer (GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glNormalPointer (type, stride, pointer);
  }

  GLuint context::genLists (GLsizei range)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGenLists (range);
  }
  void context::deleteLists (GLuint list, GLsizei range)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDeleteLists (list, range);
  }
  void context::newList (GLuint list, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glNewList (list, mode);
  }
  void context::endList()
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glEndList();
  }
  void context::callList (GLuint list)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glCallList (list);
  }

  void context::genPrograms (GLsizei count, GLuint* programs)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGenProgramsARB (count, programs);
  }
  void context::deletePrograms (GLsizei count, GLuint* programs)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDeleteProgramsARB (count, programs);
  }
  void context::bindProgram (GLenum target, GLuint program)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBindProgramARB (target, program);
  }
  void context::programString (GLenum target, GLenum format, GLsizei len, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _
      ( BOOST_CURRENT_FUNCTION
      , [this]
        {
          GLint error_position;
          getIntegerv (GL_PROGRAM_ERROR_POSITION_ARB, &error_position);
          return " at " + std::to_string (error_position) + ": " + reinterpret_cast<char const*> (getString (GL_PROGRAM_ERROR_STRING_ARB));
        }
      );
    return glProgramStringARB (target, format, len, pointer);
  }
  void context::getProgramiv (GLuint program, GLenum pname, GLint* params)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGetProgramiv (program, pname, params);
  }
  void context::programLocalParameter4f (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glProgramLocalParameter4fARB (target, index, x, y, z, w);
  }

  void context::lightf (GLenum light, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLightf (light, pname, param);
  }
  void context::lighti (GLenum light, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLighti (light, pname, param);
  }
  void context::lightfv (GLenum light, GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLightfv (light, pname, param);
  }
  void context::lightiv (GLenum light, GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLightiv (light, pname, param);
  }
  void context::lightModelf (GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLightModelf (pname, param);
  }
  void context::lightModeli (GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLightModeli (pname, param);
  }
  void context::lightModelfv (GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLightModelfv (pname, param);
  }
  void context::lightModeliv (GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glLightModeliv (pname, param);
  }

  void context::materiali (GLenum face, GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMateriali (face, pname, param);
  }
  void context::materialf (GLenum face, GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMaterialf (face, pname, param);
  }
  void context::materialiv (GLenum face, GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMaterialiv (face, pname, param);
  }
  void context::materialfv (GLenum face, GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glMaterialfv (face, pname, param);
  }
  void context::colorMaterial (GLenum face, GLenum mode)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColorMaterial (face, mode);
  }

  void context::fogi (GLenum pname, GLint param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glFogi (pname, param);
  }
  void context::fogiv (GLenum pname, GLint const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glFogiv (pname, param);
  }
  void context::fogf (GLenum pname, GLfloat param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glFogf (pname, param);
  }
  void context::fogfv (GLenum pname, GLfloat const* param)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glFogfv (pname, param);
  }

  void context::getBooleanv (GLenum target, GLboolean* value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGetBooleanv (target, value);
  }
  void context::getDoublev (GLenum target, GLdouble* value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGetDoublev (target, value);
  }
  void context::getFloatv (GLenum target, GLfloat* value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGetFloatv (target, value);
  }
  void context::getIntegerv (GLenum target, GLint* value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGetIntegerv (target, value);
  }

  GLubyte const* context::getString (GLenum target)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGetString (target);
  }

  GLuint context::createShader (GLenum shader_type)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glCreateShader (shader_type);
  }
  void context::deleteShader (GLuint shader)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDeleteShader (shader);
  }
  void context::shaderSource (GLuint shader, GLsizei count, GLchar const** string, GLint const* length)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glShaderSource (shader, count, string, length);
  }
  void context::compile_shader (GLuint shader)
  {
    {
      verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
      glCompileShader (shader);
    }
    if (get_shader (shader, GL_COMPILE_STATUS) != GL_TRUE)
    {
      std::vector<char> log (get_shader (shader, GL_INFO_LOG_LENGTH));
      glGetShaderInfoLog (shader, log.size(), nullptr, log.data());
      LogDebug << std::string (log.data ()) << std::endl;
      throw std::runtime_error ("compiling shader failed: " + std::string (log.data()));
    }
  }
  GLint context::get_shader (GLuint shader, GLenum pname)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    GLint params;
    glGetShaderiv (shader, pname, &params);
    return params;
  }

  GLuint context::createProgram()
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glCreateProgram();
  }
  void context::deleteProgram (GLuint program)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDeleteProgram (program);
  }
  void context::attachShader (GLuint program, GLuint shader)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glAttachShader (program, shader);
  }
  void context::detachShader (GLuint program, GLuint shader)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDetachShader (program, shader);
  }
  void context::link_program (GLuint program)
  {
    {
      verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
      glLinkProgram (program);
    }
    if (get_program (program, GL_LINK_STATUS) != GL_TRUE)
    {
      std::vector<char> log (get_program (program, GL_INFO_LOG_LENGTH));
      glGetProgramInfoLog (program, log.size(), nullptr, log.data());
      throw std::runtime_error ("linking program failed: " + std::string (log.data()));
    }
  }
  void context::useProgram (GLuint program)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUseProgram (program);
  }
  void context::validate_program (GLuint program)
  {
    {
      verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
      glValidateProgram (program);
    }
    if (get_program (program, GL_VALIDATE_STATUS) != GL_TRUE)
    {
      //! \todo show log
      throw std::runtime_error ("validating program failed");
    }
  }
  GLint context::get_program (GLuint program, GLenum pname)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    GLint params;
    glGetProgramiv (program, pname, &params);
    return params;
  }

  GLint context::getAttribLocation (GLuint program, GLchar const* name)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGetAttribLocation (program, name);
  }
  void context::vertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid const* pointer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glVertexAttribPointer (index, size, type, normalized, stride, pointer);
  }
  void context::enableVertexAttribArray (GLuint index)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glEnableVertexAttribArray (index);
  }
  void context::disableVertexAttribArray (GLuint index)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glDisableVertexAttribArray (index);
  }

  GLint context::getUniformLocation (GLuint program, GLchar const* name)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    auto val (glGetUniformLocation (program, name));
    if (val == -1)
    {
      throw std::logic_error ("unknown uniform " + std::string (name));
    }
    return val;
  }

  void context::uniform1i (GLint location, GLint value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUniform1i (location, value);
  }
  void context::uniform1ui (GLint location, GLuint value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUniform1ui (location, value);
  }
  void context::uniform1f (GLint location, GLfloat value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUniform1f (location, value);
  }

  void context::uniform1iv (GLint location, GLsizei count, GLint const* value)
  {
    verify_context_and_check_for_gl_errors const _(BOOST_CURRENT_FUNCTION);
    return glUniform1iv(location, count, value);
  }

  void context::uniform3fv (GLint location, GLsizei count, GLfloat const* value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUniform3fv (location, count, value);
  }
  void context::uniform4fv (GLint location, GLsizei count, GLfloat const* value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUniform4fv (location, count, value);
  }
  void context::uniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, GLfloat const* value)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glUniformMatrix4fv (location, count, transpose, value);
  }

  void context::clearStencil (GLint s)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glClearStencil (s);
  }
  void context::stencilFunc (GLenum func, GLint ref, GLuint mask)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glStencilFunc (func, ref, mask);
  }
  void context::stencilOp (GLenum sfail, GLenum dpfail, GLenum dppass)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glStencilOp (sfail, dpfail, dppass);
  }
  void context::colorMask (GLboolean r, GLboolean g, GLboolean b, GLboolean a)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glColorMask (r, g, b, a);
  }

  void context::polygonOffset (GLfloat factor, GLfloat units)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPolygonOffset (factor, units);
  }

  void context::pushAttrib (GLbitfield mask)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPushAttrib (mask);
  }
  void context::popAttrib()
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glPopAttrib();
  }

  void context::genFramebuffers (GLsizei n, GLuint *ids)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGenFramebuffers (n, ids);
  }
  void context::bindFramebuffer (GLenum target, GLuint framebuffer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBindFramebuffer (target, framebuffer);
  }
  void context::framebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glFramebufferTexture2D (target, attachment, textarget, texture, level);
  }

  void context::genRenderbuffers (GLsizei n, GLuint *ids)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glGenRenderbuffers (n, ids);
  }
  void context::bindRenderbuffer (GLenum target, GLuint renderbuffer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glBindRenderbuffer (target, renderbuffer);
  }
  void context::renderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glRenderbufferStorage (target, internalformat, width, height);
  }
  void context::framebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
  {
    verify_context_and_check_for_gl_errors const _ (BOOST_CURRENT_FUNCTION);
    return glFramebufferRenderbuffer (target, attachment, renderbuffertarget, renderbuffer);
  }

  template<GLenum target>
    void context::bufferData (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage)
  {
    scoped::buffer_binder<target> const _ (buffer);
    return bufferData (target, size, data, usage);
  }
  template void context::bufferData<GL_ARRAY_BUFFER> (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);

  void context::vertexPointer (GLuint buffer, GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    scoped::buffer_binder<GL_ARRAY_BUFFER> const _ (buffer);
    return vertexPointer (size, type, stride, pointer);
  }
  void context::colorPointer (GLuint buffer, GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    scoped::buffer_binder<GL_ARRAY_BUFFER> const _ (buffer);
    return colorPointer (size, type, stride, pointer);
  }
  void context::texCoordPointer (GLuint buffer, GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    scoped::buffer_binder<GL_ARRAY_BUFFER> const _ (buffer);
    return texCoordPointer (size, type, stride, pointer);
  }
  void context::normalPointer (GLuint buffer, GLenum type, GLsizei stride, GLvoid const* pointer)
  {
    scoped::buffer_binder<GL_ARRAY_BUFFER> const _ (buffer);
    return normalPointer (type, stride, pointer);
  }

  void context::drawElements (GLenum mode, GLuint index_buffer, GLsizei count, GLenum type, GLvoid const* indices)
  {
    scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (index_buffer);
    return drawElements (mode, count, type, indices);
  }
}
