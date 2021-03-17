#include <noggit/scripting/script.h>

#include <string>
#include <map>

namespace noggit {
  namespace scripting {
    static std::vector<script> scripts;

    script::script(std::string const& name, sol::protected_function select_event)
      : _name(name)
      : _select_event(select_event)
      {};

    void script::set_name(std::string const& name)
    {
      _name(name);
    }

    void script::on_click(sol::protected_function listener)
    {
      _click_event(listener);
    }

    std::string script::get_name()
    {
      return _name;
    }

    void register_script(std::string const& id, std::string const& name)
    {
      scripts[id] = script();
    }

    script * get_script(int id)
    {
      return scripts[id];
    }

    bool has_script(std::string const& name)
    {
      for(auto & script : scripts)
      {
        if(script.get_name() == name)
      }
      return scripts.find(id) != scripts.end();
    }
    
    void clear_scripts()
    {
      scripts.clear();
    }
  }
}