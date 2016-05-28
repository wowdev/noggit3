// call_list.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <opengl/context.hpp>
#include <opengl/call_list.h>

namespace opengl
{
  call_list::call_list()
  {
    list = gl.genLists (1);
  }
  call_list::~call_list()
  {
    gl.deleteLists (list, 1);
  }

  void call_list::start_recording (mode_type mode)
  {
    gl.newList (list, mode);
  }
  void call_list::end_recording()
  {
    gl.endList();
  }
  void call_list::render()
  {
    gl.callList (list);
  }
}
