#pragma once

#include <math/vector_3d.hpp>

namespace noggit {
  namespace scripting {
    class scripting_tool;
    void register_global(sol::state * state, scripting_tool * tool);
  }
}