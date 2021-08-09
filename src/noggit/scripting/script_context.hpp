// This file is part of Noggit3, licensed under GNU General Public License (version 3).
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

    class script_context: public sol::state
    {
    public:
      script_context(scripting_tool * tool);
      void select_script(int index);
      int get_selection();
      World * world();
      scripting_tool * tool();
      std::string get_selected_name();
      std::vector<std::shared_ptr<script_brush>> &get_scripts();
      sol::table require(std::string const& path);
      void execute_file(std::string const& filename);
      std::string file_to_module(std::string const& file);
      std::string module_to_file(std::string const& module);
    private:
      scripting_tool * _tool;
      std::vector<std::shared_ptr<script_brush>> _scripts;
      std::map<std::string, sol::table> _modules;
      std::vector<std::string> _file_stack;
      int _selected = -1;
    };

    template <typename T>
    class script_scoped_function
    {
    public:
      script_scoped_function( script_context *lua
                            , std::string const &name
                            , std::function<T> fun
                            )
                            : _lua(lua)
                            , _name(name)
      {
        lua->set_function(name, fun);
      }

      script_scoped_function()
      {
        _lua->set_function(_name, nullptr);
      }
    private:
      script_context* _lua;
      std::string _name;
    };
  } // namespace scripting
} // namespace noggit