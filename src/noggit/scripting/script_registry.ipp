#pragma once

#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/script_global.hpp>
#include <noggit/scripting/script_vert_iterator.hpp>
#include <noggit/scripting/script_tex_iterator.hpp>
#include <noggit/scripting/script_tex.hpp>
#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_chunk.hpp>
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/script_standard_brush.hpp>
#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    namespace {
      void register_functions(sol::state * lua, noggit::scripting::scripting_tool * tool)
      {
        // Register your functions / add registry functions here!
        register_script_brush(lua, tool);
        register_global(lua, tool);
        register_vert_iterator(lua, tool);
        register_tex_iterator(lua, tool);
        register_vert(lua, tool);
        register_chunk(lua, tool);
        register_selection(lua, tool);
        register_random(lua, tool);
        register_noise(lua, tool);
        register_math(lua, tool);
        register_image(lua, tool);
        register_filesystem(lua, tool);
        register_standard_brush(lua, tool);
        register_tex(lua, tool);
      }
    }
  }
}
