// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_object.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>

#include <noggit/World.h>

namespace noggit {
  namespace scripting {
      script_object::script_object(script_context * state)
      : _state(state)
      {}

      sol::object script_object::set(const std::string& key, sol::stack_object obj)
      {
        initialize_table();
        _table[key] = obj;
        return obj;
      }

      bool script_object::has_table()
      {
        return _initialized;
      }

      World * script_object::world()
      {
        return _state->world();
      }

      void script_object::initialize_table()
      {
        if(!_initialized)
        {
          _table = _state->create_table();
          _initialized = true;
        }
      }

      sol::object script_object::get(const std::string& key)
      {
        initialize_table();
        return _table[key];
      }

      sol::table script_object::table()
      {
        initialize_table();
        return _table;
      }

      script_context * script_object::state()
      {
        return _state;
      }

      scripting_tool * script_object::tool()
      {
        return _state->tool();
      }
  }
}