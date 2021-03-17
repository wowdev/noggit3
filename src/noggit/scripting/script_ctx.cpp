#include <noggit/scripting/script_ctx.hpp>
#include <boost/scope_exit.hpp>

#include <noggit/scripting/script_ctx-function_registry.ipp>

namespace noggit
{
  namespace scripting
  {
    script_scoped_function::script_scoped_function(
        sol::state * lua
      , std::string const& name
      , sol::protected_function fun)
      : _lua(lua)
      , _name(name)
      {
        lua->set_function(name,fun);
      }
    script_scoped_function::~script_scoped_function()
    {
      lua->set_function(name,nullptr);
    }

    script_context::~script_context()
    {
      delete _lua;
    }

    sol::state *script_context::get_state()
    {
      return _lua;
    }

    std::vector<script> script_context::get_scripts()
    {
      return _scripts;
    }

    void select_script(int index)
    {
      _selected = index;
    }

    int get_selection()
    {
      return _selected;
    }

    script_context::reset(World* world, noggit::camera * camera)
    {
      // TODO: can you do this without deleting it?
      if (lua != nullptr)
      {
        delete lua;
      }
      lua = new sol::state();
      _scripts.clear();
      _selected = -1;
      std::string old_name = _selected > 0 ? _scripts[_selected] : "";

      script_scoped_function add_script(lua,"add_script",
        [this](std::string const& name, sol::protected_function select_event)
        {
          this->get_scripts.push_back(script(name,select_event));
        });

      register_state(state, world, camera);

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
          lua->script_file(file);
        }
        catch (std::exception e)
        {
          get_cur_tool()->setStyleSheet("background-color: #f0a5a5;");
          get_cur_tool()->addLog("Script error:" + std::string(e.what()));
          get_cur_tool()->resetLogScroll();
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