// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/types.hpp>

#include <QtGui/QOpenGLFunctions_3_3_Core>

namespace opengl
{
  // The caller guarantees that the index buffer is already bound.
  struct index_buffer_is_already_bound{};

  struct context
  {
    struct scoped_setter
    {
      scoped_setter (context&, QOpenGLContext*);
      ~scoped_setter();

      scoped_setter (scoped_setter const&) = delete;
      scoped_setter (scoped_setter&&) = delete;
      scoped_setter& operator= (scoped_setter const&) = delete;
      scoped_setter& operator= (scoped_setter&&) = delete;

    private:
      context& _context;
      QOpenGLContext* _old_context;
      QOpenGLFunctions_3_3_Core* _old_core_func;
    };

    struct save_current_context
    {
      save_current_context (context&);
      ~save_current_context();

      save_current_context (save_current_context const&) = delete;
      save_current_context (save_current_context&&) = delete;
      save_current_context& operator= (save_current_context const&) = delete;
      save_current_context& operator= (save_current_context&&) = delete;

    private:
      bool _is_current;
      QOpenGLContext* _gl_context;
      QSurface* _surface;
    };

    QOpenGLContext* _current_context = nullptr;
    QOpenGLFunctions_3_3_Core* _3_3_core_func = nullptr;

    void enable (GLenum);
    void disable (GLenum);
    GLboolean isEnabled (GLenum);

    void viewport (GLint x, GLint y, GLsizei width, GLsizei height);

    void depthFunc (GLenum);
    void depthMask (GLboolean);
    void blendFunc (GLenum, GLenum);

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

    void genTextures (GLuint, GLuint*);
    void deleteTextures (GLuint, GLuint*);
    void bindTexture (GLenum target, GLuint);
    void texImage2D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid const* data);
    void compressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid const* data);
    void generateMipmap (GLenum);
    void activeTexture (GLenum);

    void texParameteri (GLenum target, GLenum pname, GLint param);
    void texParameterf (GLenum target, GLenum pname, GLfloat param);
    void texParameteriv (GLenum target, GLenum pname, GLint const* params);
    void texParameterfv (GLenum target, GLenum pname, GLfloat const* params);

    void genVertexArrays (GLuint, GLuint*);
    void deleteVertexArray (GLuint, GLuint*);
    void bindVertexArray (GLenum);

    void genBuffers (GLuint, GLuint*);
    void deleteBuffers (GLuint, GLuint*);
    void bindBuffer (GLenum, GLuint);
    void bufferData (GLenum target, GLsizeiptr size, GLvoid const* data, GLenum usage);
    GLvoid* mapBuffer (GLenum target, GLenum access);
    GLboolean unmapBuffer (GLenum);

    // The drawElements() family uses indices. These indices can either
    // - be already bound (`type, opengl::index_buffer_is_already_bound{}`, not checked, caller has to ensure),
    // - automatically be bound from the buffer given (`type, index_buffer`), or
    // - [TRY TO AVOID] be uploaded by the drawElements() function via a CPU buffer (`indices`).
    //
    // Regardless of the way chosen to specify the indices, indices_offset can be used to
    // point into the buffer, e.g. to skip vertices also in the buffer.
    //
    // \note Not all combinations are implemented for sake of implementer's time.
    // \todo Take index information via some object that encapsulates the mess.

    void drawElements (GLenum mode, GLsizei count, GLenum type, GLuint index_buffer,           std::intptr_t indices_offset = 0);
    void drawElements (GLenum mode, GLsizei count, GLenum type, index_buffer_is_already_bound, std::intptr_t indices_offset = 0);
    template<typename T>
      void drawElements (GLenum mode, GLsizei count, std::vector<T> const& indices,            std::intptr_t indices_offset = 0);
    void drawElementsInstanced (GLenum mode, GLsizei count, GLsizei instancecount, GLenum type, index_buffer_is_already_bound, std::intptr_t indices_offset = 0);
    void drawElementsInstanced (GLenum mode, GLsizei count, GLsizei instancecount, GLenum type, GLuint index_buffer,           std::intptr_t indices_offset = 0);
    template<typename T>
      void drawElementsInstanced (GLenum mode, GLsizei count, GLsizei instancecount, std::vector<T> const& indices,            std::intptr_t indices_offset = 0);
    void drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, index_buffer_is_already_bound, std::intptr_t indices_offset = 0);

    void genPrograms (GLsizei programs, GLuint*);
    void deletePrograms (GLsizei programs, GLuint*);
    void bindProgram (GLenum, GLuint);
    void programString (GLenum target, GLenum format, GLsizei len, GLvoid const* pointer);
    void getProgramiv (GLuint program, GLenum pname, GLint* params);
    void programLocalParameter4f (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);

    void getBooleanv (GLenum, GLboolean*);
    void getDoublev (GLenum, GLdouble*);
    void getFloatv (GLenum, GLfloat*);
    void getIntegerv (GLenum, GLint*);

    GLubyte const* getString (GLenum);

    GLuint createShader (GLenum shader_type);
    void deleteShader (GLuint shader);
    void shaderSource (GLuint shader, GLsizei count, GLchar const** string, GLint const* length);
    void compile_shader (GLuint shader);
    GLint get_shader (GLuint shader, GLenum pname);

    GLuint createProgram();
    void deleteProgram (GLuint program);
    void attachShader (GLuint program, GLuint shader);
    void detachShader (GLuint program, GLuint shader);
    void link_program (GLuint program);
    void useProgram (GLuint program);
    void validate_program (GLuint program);
    GLint get_program (GLuint program, GLenum pname);
    std::string get_program_info_log(GLuint program);

    GLint getAttribLocation (GLuint program, GLchar const* name);
    void vertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid const* pointer);
    void vertexAttribDivisor (GLuint index, GLuint divisor);
    void enableVertexAttribArray (GLuint index);
    void disableVertexAttribArray (GLuint index);

    GLint getUniformLocation (GLuint program, GLchar const* name);
    void uniform1i (GLint location, GLint value);
    void uniform1f (GLint location, GLfloat value);
    void uniform1iv (GLint location, GLsizei count, GLint const* value);
    void uniform2fv (GLint location, GLsizei count, GLfloat const* value);
    void uniform3fv (GLint location, GLsizei count, GLfloat const* value);
    void uniform4fv (GLint location, GLsizei count, GLfloat const* value);
    void uniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, GLfloat const* value);

    void clearStencil (GLint);
    void stencilFunc (GLenum func, GLint ref, GLuint mask);
    void stencilOp (GLenum sfail, GLenum dpfail, GLenum dppass);
    void colorMask (GLboolean r, GLboolean g, GLboolean b, GLboolean a);

    void polygonOffset (GLfloat factor, GLfloat units);

    void genFramebuffers (GLsizei n, GLuint *ids);
    void bindFramebuffer (GLenum target, GLuint framebuffer);
    void framebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

    void genRenderbuffers (GLsizei n, GLuint *renderbuffers);
    void bindRenderbuffer (GLenum target, GLuint renderbuffer);
    void renderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void framebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

    template<GLenum target>
      void bufferData (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);
    template<GLenum target, typename T>
      void bufferData(GLuint buffer, std::vector<T> const& data, GLenum usage);
  };
}

extern opengl::context gl;
