#pragma once

#include <noggit/scripting/script_brush.hpp>
#include <sol/sol.hpp>

#include <functional>

class World;

namespace noggit {
  class camera;
  namespace scripting {
    class scripting_tool;
    // function that should only exist through some scope
    // TODO: add move constructors etc

    template <typename T>
    class script_scoped_function {
    public:
      script_scoped_function(
        sol::state* lua
        , std::string const& name
        , std::function<T> fun)
        : _lua(lua)
        , _name(name)
        {
          lua->set_function(name, fun);
        }
      ~script_scoped_function()
      {
        _lua->set_function(_name, nullptr);
      }
    private:
      sol::state * _lua;
      std::string _name;
    };

    class script_context {
    public:
      script_context() = default;
      ~script_context();
      sol::state * get_state();
      std::vector<noggit::scripting::script_brush>& get_scripts();
      void reset(noggit::scripting::scripting_tool * tool);
      void select_script(int index);
      int get_selection();
      std::string get_selected_name();
    private:
      sol::state * _lua = new sol::state();
      std::vector<noggit::scripting::script_brush> _scripts;
      int _selected = -1;
    };
  }
}