// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/script_global.hpp>
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
#include <noggit/scripting/script_settings.hpp>
#include <noggit/scripting/script_procedures.hpp>

#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    namespace {
      void register_functions(script_context * lua)
      {
        // Register your functions / add registry functions here!
        register_script_brush(lua);
        register_model(lua);
        register_global(lua);
        register_vert(lua);
        register_tex(lua);
        register_chunk(lua);
        register_selection(lua);
        register_random(lua);
        register_noise(lua);
        register_math(lua);
        register_image(lua);
        register_filesystem(lua);
        register_standard_brush(lua);
        register_procedures(lua);
        register_settings(lua);
      }
    }
  }
}
