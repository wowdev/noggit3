#pragma once

#include <math/vector_3d.hpp>

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class script_context;
    void register_global(script_context * state);
  }
}