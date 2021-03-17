#include <noggit/scripting/script_brush.hpp>

#include <string>
#include <map>

namespace noggit {
  namespace scripting {
    script_brush::script_brush(std::string const& name, sol::protected_function select_event)
      : _name(name)
      , _select_event(select_event)
      {};

    void script_brush::set_name(std::string const& name)
    {
      _name = name;
    }

    void script_brush::on_click(sol::protected_function listener)
    {
      _click_event = listener;
    }

    std::string script_brush::get_name()
    {
      return _name;
    }
  }
}