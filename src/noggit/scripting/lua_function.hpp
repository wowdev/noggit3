#pragma once

#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    template <typename ...Ts>
    class lua_function {
    public:
      lua_function()
        : _is_null(true)
      {

      }
      lua_function(sol::protected_function fn)
        : _is_null(false)
        , _fn(fn)
      {}

      void call_if_not_null(std::string caller, Ts...args)
      {
        if(!_is_null) call(caller, args...);
      }

      void call(std::string caller, Ts...args)
      {
        if(_is_null)
        {
          throw script_exception(caller,"calling null function");
        }
        auto ret = _fn(args...);
        if (!ret.valid())
        {
          sol::error err = ret;
          throw script_exception(caller, err.what());
        }
      }

      bool is_null()
      {
        return _is_null;
      }

    private:
      sol::protected_function _fn;
      bool _is_null;
    };

    template <typename T, typename ...Ts>
    class lua_function_ret {
    public:
      lua_function_ret()
        : _is_null(true)
        {}
      lua_function_ret(sol::type type, sol::protected_function fn)
        : _is_null(false)
        , _type(type)
        , _fn(fn)
      {}

      T call_if_not_null(std::string caller, T defValue, Ts...args)
      {
        if(_is_null)
        {
          return defValue;
        }
        return call(caller, ...args);
      }

      T call(std::string caller, Ts...args)
      {
        if(_is_null)
        {
          throw script_exception(caller, "calling null function");
        }
        auto ret = _fn(args...);
        if (!ret.valid())
        {
          sol::error err = ret;
          throw script_exception(caller, err.what());
        }

        if (ret.get_type() != _type)
        {
          // TODO: state the expected/received types
          throw script_exception(caller, "incorrect return type");
        }
        return ret.get<T>();
      }

      bool is_null()
      {
        return _is_null;
      }
    private:
      bool _is_null;
      sol::type _type;
      sol::protected_function _fn;
    };
  }
}