#pragma once

#include <noggit/scripting/script_brush.hpp>
#include <sol/sol.hpp>

#include <functional>
#include <map>

class World;

namespace noggit
{
  class camera;
  namespace scripting
  {
    class scripting_tool;


    template <typename T>
    class script_scoped_function
    {
    public:
      script_scoped_function(
          sol::state *lua, std::string const &name, std::function<T> fun)
          : _lua(lua), _name(name)
      {
        lua->set_function(name, fun);
      }
      ~script_scoped_function()
      {
        _lua->set_function(_name, nullptr);
      }

    private:
      sol::state *_lua;
      std::string _name;
    };

    enum class script_module_state
    {
      UNLOADED,
      LOADING,
      ERRORED,
      LOADED
    };

    class script_module
    {
    public:
      script_module() = default;
      script_module(sol::function fn);
      sol::table require();

    private:
      script_module_state _state;
      sol::table _table;
      lua_function_ret<sol::table> _fn;
    };

    class script_context
    {
    public:
      script_context() = default;
      ~script_context();
      sol::state *get_state();
      void reset(noggit::scripting::scripting_tool *tool);
      void select_script(int index);
      int get_selection();
      std::string get_selected_name();
      std::vector<noggit::scripting::script_brush> &get_scripts();

      std::map<std::string, script_module> _modules;
    private:
      sol::state *_lua = new sol::state();

      std::vector<noggit::scripting::script_brush> _scripts;

      int _selected = -1;
    };
  } // namespace scripting
} // namespace noggit