// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtOpenGL>

namespace opengl
{
  struct context
  {
    struct scoped_setter
    {
      scoped_setter (context& context_, QOpenGLContext* current_context)
        : _context (context_)
        , _old_context (_context._current_context)
      {
        _context._current_context = current_context;
      }
      ~scoped_setter()
      {
        _context._current_context = _old_context;
      }

      scoped_setter (scoped_setter const&) = delete;
      scoped_setter (scoped_setter&&) = delete;
      scoped_setter& operator= (scoped_setter const&) = delete;
      scoped_setter& operator= (scoped_setter&&) = delete;

    private:
      context& _context;
      QOpenGLContext* _old_context;
    };

    QOpenGLContext* _current_context = nullptr;

    void enable (GLenum);
    void disable (GLenum);
    GLboolean isEnabled (GLenum);

    void begin (GLenum);
    void end();

    void enableClientState (GLenum);
    void disableClientState (GLenum);
    void clientActiveTexture (GLenum);

    void normal3f (GLfloat, GLfloat, GLfloat);
    void normal3fv (GLfloat const[3]);
    void vertex2f (GLfloat, GLfloat);
    void vertex3f (GLfloat, GLfloat, GLfloat);
    void vertex3fv (GLfloat const[3]);
    void color3f (GLfloat, GLfloat, GLfloat);
    void color4f (GLfloat, GLfloat, GLfloat, GLfloat);
    void color4ub (GLubyte, GLubyte, GLubyte, GLubyte);
    void color3fv (GLfloat const[3]);
    void color4fv (GLfloat const[4]);
    void texCoord2f (GLfloat, GLfloat);
    void texCoord2fv (GLfloat const[2]);
    void multiTexCoord2f (GLenum target, GLfloat, GLfloat);

    void matrixMode (GLenum);
    void pushMatrix();
    void popMatrix();
    void loadIdentity();
    void translatef (GLfloat, GLfloat, GLfloat);
    void scalef (GLfloat, GLfloat, GLfloat);
    void rotatef (GLfloat, GLfloat, GLfloat, GLfloat);
    void multMatrixf (GLfloat const[4]);

    void ortho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);
    void frustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);
    void viewport (GLint x, GLint y, GLsizei width, GLsizei height);

    void alphaFunc (GLenum, GLfloat);
    void depthFunc (GLenum);
    void depthMask (GLboolean);
    void blendFunc (GLenum, GLenum);
    void shadeModel (GLenum);

    void clear (GLenum);
    void clearColor (GLfloat, GLfloat, GLfloat, GLfloat);

    void readBuffer (GLenum);
    void readPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data);

    void lineWidth (GLfloat);

    void pointParameterf (GLenum pname, GLfloat param);
    void pointParameteri (GLenum pname, GLint param);
    void pointParameterfv (GLenum pname, GLfloat const* param);
    void pointParameteriv (GLenum pname, GLint const* param);
    void pointSize (GLfloat);

    void hint (GLenum, GLenum);
    void polygonMode (GLenum face, GLenum mode);
    GLint renderMode (GLenum mode);

    void genTextures (GLuint, GLuint*);
    void deleteTextures (GLuint, GLuint*);
    void bindTexture (GLenum target, GLuint);
    void texImage2D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid const* data);
    void compressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid const* data);
    void generateMipmap (GLenum);
    void activeTexture (GLenum);

    void texEnvf (GLenum, GLenum, GLfloat);
    void texEnvi (GLenum, GLenum, GLint);

    void texGeni (GLenum coord, GLenum pname, GLint param);
    void texGenf (GLenum coord, GLenum pname, GLfloat param);
    void texGend (GLenum coord, GLenum pname, GLdouble param);
    void texGeniv (GLenum coord, GLenum pname, GLint const* params);
    void texGenfv (GLenum coord, GLenum pname, GLfloat const* params);
    void texGendv (GLenum coord, GLenum pname, GLdouble const* params);

    void texParameteri (GLenum target, GLenum pname, GLint param);
    void texParameterf (GLenum target, GLenum pname, GLfloat param);
    void texParameteriv (GLenum target, GLenum pname, GLint const* params);
    void texParameterfv (GLenum target, GLenum pname, GLfloat const* params);

    void genBuffers (GLuint, GLuint*);
    void deleteBuffers (GLuint, GLuint*);
    void bindBuffer (GLenum, GLuint);
    void bufferData (GLenum target, GLsizeiptr size, GLvoid const* data, GLenum usage);
    GLvoid* mapBuffer (GLenum target, GLenum access);
    GLboolean unmapBuffer (GLenum);
    void drawElements (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices);
    void drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid const* indices);

    void vertexPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer);
    void colorPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer);
    void texCoordPointer (GLint size, GLenum type, GLsizei stride, GLvoid const* pointer);
    void normalPointer (GLenum type, GLsizei stride, GLvoid const* pointer);

    GLuint genLists (GLsizei);
    void deleteLists (GLuint list, GLsizei range);
    void newList (GLuint list, GLenum mode);
    void endList();
    void callList (GLuint list);

    void genPrograms (GLsizei programs, GLuint*);
    void deletePrograms (GLsizei programs, GLuint*);
    void bindProgram (GLenum, GLuint);
    void programString (GLenum target, GLenum format, GLsizei len, GLvoid const* pointer);
    void getProgramiv (GLuint program, GLenum pname, GLint* params);
    void programLocalParameter4f (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);

    void initNames();
    void pushName (GLuint);
    void popName();
    void selectBuffer (GLsizei size, GLuint* buffer);

    void lightf (GLenum light, GLenum pname, GLfloat param);
    void lighti (GLenum light, GLenum pname, GLint param);
    void lightfv (GLenum light, GLenum pname, GLfloat const* param);
    void lightiv (GLenum light, GLenum pname, GLint const* param);
    void lightModelf (GLenum pname, GLfloat param);
    void lightModeli (GLenum pname, GLint param);
    void lightModelfv (GLenum pname, GLfloat const* param);
    void lightModeliv (GLenum pname, GLint const* param);

    void materiali (GLenum, GLenum, GLint);
    void materialf (GLenum, GLenum, GLfloat);
    void materialiv (GLenum, GLenum, GLint const*);
    void materialfv (GLenum, GLenum, GLfloat const*);
    void colorMaterial (GLenum, GLenum);

    void fogi (GLenum, GLint);
    void fogiv (GLenum, GLint const*);
    void fogf (GLenum, GLfloat);
    void fogfv (GLenum, GLfloat const*);

    void getBooleanv (GLenum, GLboolean*);
    void getDoublev (GLenum, GLdouble*);
    void getFloatv (GLenum, GLfloat*);
    void getIntegerv (GLenum, GLint*);

    GLubyte const* getString (GLenum);
  };
}

extern opengl::context gl;
