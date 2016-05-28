// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <opengl/context.h>
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
