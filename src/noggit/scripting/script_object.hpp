// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_exception.hpp>

#include <sol/sol.hpp>

class World;

namespace noggit {
  namespace scripting {
    class script_context;
    class scripting_tool;

    class script_object {
    public:
      // TODO: We can probably get this in a more intelligent way
      script_object(script_context * state);
      virtual sol::object set(const std::string& key, sol::stack_object obj); 
      virtual sol::object get(const std::string& key); 

      script_context * state();
      scripting_tool * tool();
      World * world();
      sol::table table();
      bool has_table();
    protected:
      sol::table _table;
      void initialize_table();
    private:
      bool _initialized = false;
      script_context * _state;
    };

    template <typename O, typename ...Ts>
    class lua_function {
    public:
      lua_function(O * obj, std::string const& func)
        : _obj(obj)
        , _func(func)
      {}

      bool exists()
      {
        if (!_obj->has_table())
        {
          return false;
        }

        auto fn = _obj->table()[_func];
        if (!fn.valid())
        {
          return false;
        }

        if (fn.get_type() == sol::type::function)
        {
          return true;
        }
        return false;
      }

      void call_if_exists(std::string caller, Ts...args)
      {
        if (exists()) call(caller, args...);
      }

      void call(std::string caller, Ts...args)
      {
        if(!exists())
        {
          throw script_exception(caller,"calling null function");
        }
        sol::protected_function fn = _obj->table()[_func];
        auto ret = fn(_obj, args...);
        if (!ret.valid())
        {
          sol::error err = ret;
          throw script_exception(caller, err.what());
        }
      }
    private:
      O * _obj;
      std::string _func;
    };

    #define LUA_MEMBER_FUNC(selftype, type,name) lua_function<selftype, type> name = lua_function<selftype, type>(this,#name);
  }
}