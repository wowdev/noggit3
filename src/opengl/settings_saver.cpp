// settings_saver.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <opengl/settings_saver.h>

#include <opengl/context.hpp>
#include <opengl/texture.h>

namespace opengl
{
  settings_saver::settings_saver()
  {
#define SAVE(NAME, VARNAME) _saved_settings.VARNAME = gl.isEnabled (NAME)

    SAVE (GL_ALPHA_TEST, alpha_testing);
    SAVE (GL_BLEND, blend);
    SAVE (GL_COLOR_MATERIAL, color_material);
    SAVE (GL_CULL_FACE, cull_face);
    SAVE (GL_DEPTH_TEST, depth_test);
    SAVE (GL_FOG, fog);
    SAVE (GL_FRAGMENT_PROGRAM_ARB, fragment_program);
    SAVE (GL_LIGHTING, lighting);
    SAVE (GL_LINE_SMOOTH, line_smooth);
    opengl::texture::set_active_texture (0);
    SAVE (GL_TEXTURE_2D, texture_0);
    opengl::texture::set_active_texture (1);
    SAVE (GL_TEXTURE_2D, texture_1);
    SAVE (GL_TEXTURE_GEN_S, texture_gen_s);
    SAVE (GL_TEXTURE_GEN_T, texture_gen_t);

#undef SAVE
  }

  settings_saver::~settings_saver()
  {
#define LOAD(NAME, VARNAME) if (gl.isEnabled (NAME) != _saved_settings.VARNAME) \
                            { \
                              if (_saved_settings.VARNAME == GL_TRUE) \
                              { \
                                gl.enable (NAME); \
                              } \
                              else \
                              { \
                                gl.disable (NAME); \
                              } \
                            }

    LOAD (GL_ALPHA_TEST, alpha_testing);
    LOAD (GL_BLEND, blend);
    LOAD (GL_COLOR_MATERIAL, color_material);
    LOAD (GL_CULL_FACE, cull_face);
    LOAD (GL_DEPTH_TEST, depth_test);
    LOAD (GL_FOG, fog);
    LOAD (GL_FRAGMENT_PROGRAM_ARB, fragment_program);
    LOAD (GL_LIGHTING, lighting);
    LOAD (GL_LINE_SMOOTH, line_smooth);
    opengl::texture::set_active_texture (0);
    LOAD (GL_TEXTURE_2D, texture_0);
    opengl::texture::set_active_texture (1);
    LOAD (GL_TEXTURE_2D, texture_1);
    LOAD (GL_TEXTURE_GEN_S, texture_gen_s);
    LOAD (GL_TEXTURE_GEN_T, texture_gen_t);

#undef LOAD
  }
}
