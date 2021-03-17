#pragma once

#include <noggit/scripting/script.h>
#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    class script_scoped_function {
    public:
      script_scoped_function(
        sol::state * lua
        , std::string const& name
        , sol::protected_function fun)
      ~script_scoped_function();
    private:
      sol::state * _lua;
      std::string _name;
    };

    class script_context {
    public:
      script_context() = default;
      ~script_context();
      sol::state * get_state();
      std::vector<script> get_scripts();
      void reset();
      void select_script(int index);
      int get_selection();
    private:
      sol::state _lua = new sol::state();
      std::vector<script> _scripts;
      int _selected = -1;
    };
  }
}