// call_list.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __OPENGL_CALL_LIST_H
#define __OPENGL_CALL_LIST_H

#include <gl/glew.h>

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

#endif
