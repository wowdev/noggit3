// call_list.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <opengl/call_list.h>

namespace opengl
{
  call_list::call_list()
  {
    list = glGenLists (1);
  }
  call_list::~call_list()
  {
    glDeleteLists (list, 1);
  }

  void call_list::start_recording (mode_type mode)
  {
    glNewList (list, mode);
  }
  void call_list::end_recording()
  {
    glEndList();
  }
  void call_list::render()
  {
    glCallList (list);
  }
}
