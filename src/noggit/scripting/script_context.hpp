#pragma once

#include <noggit/scripting/script_brush.hpp>
#include <sol/sol.hpp>

#include <functional>
#include <map>
#include <set>

class World;

namespace noggit
{
  class camera;
  namespace scripting
  {
    class scripting_tool;

    class lua_state : public sol::state
    {
    public:
      lua_state(scripting_tool* tool);
      scripting_tool* tool();
    private:
      scripting_tool* _tool;
    };

    template <typename T>
    class script_scoped_function
    {
    public:
      script_scoped_function(
          lua_state *lua, std::string const &name, std::function<T> fun)
          : _lua(lua), _name(name)
      {
        lua->set_function(name, fun);
      }
      ~script_scoped_function()
      {
        _lua->set_function(_name, nullptr);
      }

    private:
      lua_state * _lua;
      std::string _name;
    };

    class script_context
    {
    public:
      script_context(scripting_tool * tool);
      ~script_context();
      lua_state * get_state();
      void reset();
      void select_script(int index);
      int get_selection();
      std::string get_selected_name();
      std::vector<std::shared_ptr<noggit::scripting::script_brush>> &get_scripts();
      sol::table require(std::string const& path);
      std::map<std::string, sol::table> _modules;
      std::vector<std::string> file_stack;
      void execute_file(std::string const& filename);
      std::string file_to_module(std::string const& file);
      std::string module_to_file(std::string const& module);
    private:
      scripting_tool * _tool;
      lua_state * _lua;
      std::vector<std::shared_ptr<script_brush>> _scripts;
      int _selected = -1;
    };
  } // namespace scripting
} // namespace noggit