#pragma once

#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/script_global.hpp>
#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    namespace {
      void register_functions(sol::state * lua, noggit::scripting::scripting_tool * tool)
      {
        // Register your functions / add registry functions here!
        register_script_brush(lua, tool);
        register_global(lua, tool);
      }
    }
  }
}
