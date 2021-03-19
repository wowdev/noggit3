#pragma once

#include <math/vector_3d.hpp>

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class lua_state;
    void register_global(lua_state * state);
  }
}