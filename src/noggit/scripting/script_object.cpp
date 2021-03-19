#include <noggit/scripting/script_object.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>

namespace noggit {
  namespace scripting {
      script_object::script_object(script_context * state)
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

      bool script_object::has_table()
      {
        return _initialized;
      }

      sol::object script_object::get(const std::string& key)
      {
        return _table[key];
      }

      sol::table script_object::table()
      {
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