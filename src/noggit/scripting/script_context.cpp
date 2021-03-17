#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_registry.ipp>

#include <noggit/World.h>
#include <noggit/camera.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <vector>
namespace noggit
{
  namespace scripting
  {
    namespace {
    }

    script_context::~script_context()
    {
      delete _lua;
    }

    std::string script_context::get_selected_name()
    {
      if(_selected == -1)
      {
        return "";
      }
      return _scripts[_selected].get_name();
    }

    sol::state *script_context::get_state()
    {
      return _lua;
    }

    std::vector<noggit::scripting::script_brush> script_context::get_scripts()
    {
      return _scripts;
    }

    void script_context::select_script(int index)
    {
      _selected = index;
    }

    int script_context::get_selection()
    {
      return _selected;
    }

    void script_context::reset(noggit::scripting::scripting_tool * tool)
    {
      // TODO: can you do this without deleting it?
      if (_lua != nullptr)
      {
        delete _lua;
      }
      _lua = new sol::state();
      _scripts.clear();
      _selected = -1;
      std::string old_name = _selected > 0 ? _scripts[_selected].get_name() : "";

      script_scoped_function<void(std::string const&,sol::protected_function)> 
        add_brush(_lua,"add_brush",
        [this,tool](std::string const& name, sol::protected_function select_event)
        {
          this->get_scripts().push_back(noggit::scripting::script_brush(tool, name,select_event));
        });

      register_functions(_lua, tool);

      boost::filesystem::recursive_directory_iterator end;
      for (boost::filesystem::recursive_directory_iterator dir("scripts"); dir != end; ++dir)
      {
        std::string file = dir->path().string();
        if (!boost::ends_with(file, ".lua") || boost::ends_with(file, ".spec.lua"))
        {
          continue;
        }

        try
        {
          _lua->script_file(file);
        }
        catch (std::exception e)
        {
          tool->setStyleSheet("background-color: #f0a5a5;");
          tool->addLog("Script error:" + std::string(e.what()));
          tool->resetLogScroll();
        }
      }

      // restore the old selection if we can find the same name
      for (int i = 0; i < _scripts.size(); ++i)
      {
        if (_scripts[i].get_name() == old_name)
        {
          _selected = i;
          break;
        }
      }
      
      if(_scripts.size() > 0)
      {
        _selected = 0;
      }
    }
  } // namespace scripting
} // namespace noggit