// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtOpenGL>

namespace opengl
{
  class call_list
  {
  public:
    call_list();
    ~call_list();

    typedef GLuint mode_type;

    void start_recording (mode_type mode = GL_COMPILE);
    void end_recording();
    void render();

  private:
    typedef GLuint internal_type;
    internal_type list;
  };
}
