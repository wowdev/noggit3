#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_registry.ipp>
#include <noggit/scripting/script_exception.hpp>

#include <noggit/World.h>
#include <noggit/camera.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <noggit/Log.h>

#include <vector>
namespace noggit
{
  namespace scripting
  {

    script_module::script_module(sol::function fn)
    : _fn(lua_function_ret<sol::table>(sol::type::table,fn))
    , _state(script_module_state::UNLOADED)
    {}

    sol::table script_module::require()
    {
      switch(_state)
      {
        case script_module_state::LOADED:
          return _table;
        case script_module_state::LOADING:
          throw script_exception("require","Circular dependency in require chain");
        case script_module_state::ERRORED:
          throw script_exception("require", "Tried to require module with errors");
        case script_module_state::UNLOADED:
          _state = script_module_state::LOADING;
          try {
            _table = _fn.call("require");
          }
          catch (script_exception const& err)
          {
            _state = script_module_state::ERRORED;
            throw err;
          }
          _state = script_module_state::LOADED;
          return _table;
      }
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

    std::vector<noggit::scripting::script_brush> & script_context::get_scripts()
    {
      return _scripts;
    }

    void script_context::select_script(int index)
    {
      _selected = index;
      auto v = &get_scripts()[_selected];
      v->_select.call("(select)",*v);
    }

    int script_context::get_selection()
    {
      return _selected;
    }

    void script_context::reset(noggit::scripting::scripting_tool * tool)
    {
      std::string old_name = _selected > 0 ? _scripts[_selected].get_name() : "";
      _scripts.clear();
      _modules.clear();

      // TODO: can you do this without deleting it?
      if (_lua != nullptr)
      {
        delete _lua;
      }
      _lua = new sol::state();
      _selected = -1;

      script_scoped_function<void(std::string const&,sol::protected_function)> 
        add_brush(_lua,"brush",
        [this,tool](std::string const& name, sol::protected_function select_event)
        {
          this->get_scripts().push_back(noggit::scripting::script_brush(tool, name,select_event));
        });

      std::string * _cur_name = new std::string();
      script_scoped_function<void(sol::protected_function)> 
        _module(_lua,"module",
        [this,tool,_cur_name](sol::protected_function cb)
        {
          this->_modules[std::string(*_cur_name)] = script_module(cb);
        });

      _lua->set_function("require", [this](std::string const& name) {
        return this->_modules[name].require();
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
          auto rel = boost::filesystem::relative(dir->path(),boost::filesystem::path("scripts"))
            .string();
          std::replace(rel.begin(),rel.end(), '\\', '.');
          std::replace(rel.begin(),rel.end(), '/', '.');
          *_cur_name = rel;
          *_cur_name = _cur_name->substr(0,_cur_name->size()-strlen(".lua"));
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
      
      if(_scripts.size() > 0 && _selected < 0)
      {
        _selected = 0;
      }
    }
  } // namespace scripting
} // namespace noggit