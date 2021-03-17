#pragma once

#include <sol/sol.hpp>
#include <string>

namespace noggit {
  namespace scripting {
    class script_brush {
    public:
      script_brush(std::string const& name, sol::protected_function select_event);
      void set_name(std::string const& name);
      void on_click(sol::protected_function listener);
      std::string get_name();
    private:
      // <scripting_tool>
      sol::protected_function _select_event;
      // <click_context>
      sol::protected_function _click_event = nullptr;
      std::string _name;
    };
  }
}