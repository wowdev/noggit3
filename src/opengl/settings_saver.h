#ifndef __OPENGL_SETTINGS_SAVER_H
#define __OPENGL_SETTINGS_SAVER_H

#include <gl/glew.h>

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
