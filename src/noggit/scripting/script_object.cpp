#include <noggit/scripting/script_object.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>

namespace noggit {
  namespace scripting {
      script_object::script_object(lua_state * state)
      : _state(state)
      {}

      sol::object script_object::set(const std::string& key, sol::stack_object obj)
      {
        if(!_initialized)
        {
          _table = _state->create_table();
          _initialized = true;
        }
        _table[key] = obj;
        return obj;
      }

      sol::object script_object::get(const std::string& key)
      {
        return _table[key];
      }

      sol::table script_object::table()
      {
        return _table;
      }

      lua_state * script_object::state()
      {
        return _state;
      }

      scripting_tool * script_object::tool()
      {
        return _state->tool();
      }
  }
}