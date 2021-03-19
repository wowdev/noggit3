#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_registry.ipp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_object.hpp>

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
    lua_state::lua_state(scripting_tool * tool)
    : sol::state()
    , _tool(tool)
    {}

    scripting_tool * lua_state::tool()
    {
      return _tool;
    }
    
    script_context::script_context(scripting_tool * tool)
    : _tool(tool)
    , _lua(new lua_state(tool))
    {}

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
      return _scripts[_selected]->get_name();
    }

    lua_state * script_context::get_state()
    {
      return _lua;
    }

    std::vector<std::shared_ptr<script_brush>> & script_context::get_scripts()
    {
      return _scripts;
    }

    void script_context::select_script(int index)
    {
      _selected = index;
      auto v = get_scripts()[_selected];
      v->on_selected();
    }

    int script_context::get_selection()
    {
      return _selected;
    }

    sol::table script_context::require(std::string const& mod)
    {
      std::string err_str;
      for(auto& val: this->file_stack)
      {
        err_str+=val;
        if(val==mod)
        {
          throw script_exception("require","circular dependency: "+err_str);
        }
        err_str+="->";
      }
      if(_modules.find(mod)==_modules.end())
      {
        execute_file(module_to_file(mod));
      }
      return _modules[mod];
    }

    std::string script_context::file_to_module(std::string const& file)
    {
      auto rel = boost::filesystem::relative(boost::filesystem::path(file),boost::filesystem::path("scripts"))
        .string();
      std::replace(rel.begin(),rel.end(),'\\','/');
      return rel.substr(0,rel.size()-strlen(".lua"));
    }

    std::string script_context::module_to_file(std::string const& mod)
    {
      return (boost::filesystem::path("scripts") / boost::filesystem::path(mod + ".lua")).string();
    }

    void script_context::execute_file(std::string const& file)
    {
      auto mod = file_to_module(file);
      file_stack.push_back(mod);
      sol::protected_function_result res = _lua->script_file(file);
      _modules[mod] = res.get_type() == sol::type::table 
        ? res.get<sol::table>()
        : _lua->create_table();
      file_stack.pop_back();
    }

    void script_context::reset()
    {
      std::string old_name = _selected > 0 ? _scripts[_selected]->get_name() : "";
      _scripts.clear();
      _modules.clear();

      // TODO: can you do this without deleting it?
      if (_lua != nullptr)
      {
        delete _lua;
      }
      _lua = new lua_state(this->_tool);
      _selected = -1;

      script_scoped_function<std::shared_ptr<script_brush>(std::string const&)> 
        add_brush(_lua,"brush",
        [this](std::string const& name)
        {
          auto brush = std::make_shared<script_brush>(_lua, this->_tool,name);
          this->get_scripts().push_back(brush);
          return brush;
        });

      _lua->set_function("require", [this](std::string const& name) {
        return this->require(name);
      });

      register_functions(_lua);

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
          execute_file(dir->path().string());
        }
        catch (std::exception e)
        {
          _tool->setStyleSheet("background-color: #f0a5a5;");
          _tool->addLog("Script error:" + std::string(e.what()));
          _tool->resetLogScroll();
        }
      }

      // restore the old selection if we can find the same name
      for (int i = 0; i < _scripts.size(); ++i)
      {
        if (_scripts[i]->get_name() == old_name)
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