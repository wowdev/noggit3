// settings_saver.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __OPENGL_SETTINGS_SAVER_H
#define __OPENGL_SETTINGS_SAVER_H

#ifdef __linux__
#include <GL/glew.h>
#else
#include <gl/glew.h>
#endif

namespace opengl
{
  class settings_saver
  {
  public:
    settings_saver();
    ~settings_saver();

  private:
    struct
    {
      GLboolean alpha_testing;
      GLboolean blend;
      GLboolean color_material;
      GLboolean cull_face;
      GLboolean depth_test;
      GLboolean fog;
      GLboolean fragment_program;
      GLboolean lighting;
      GLboolean line_smooth;
      GLboolean texture_0;
      GLboolean texture_1;
      GLboolean texture_gen_s;
      GLboolean texture_gen_t;
    } _saved_settings;
  };
}

#endif
