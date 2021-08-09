// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_registry.ipp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_object.hpp>

#include <noggit/MapView.h>
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

    namespace
    {
      void set_tool_error(scripting_tool * tool)
      {
          tool->resetLogScroll();
          tool->setStyleSheet("background-color: #f0a5a5;");
      }
    }

    script_context::script_context(scripting_tool * tool): _tool(tool)
    {
      open_libraries(sol::lib::base,sol::lib::table,sol::lib::string);
      script_scoped_function<std::shared_ptr<script_brush>(std::string const&)> 
        add_brush(this,"brush",
        [this](std::string const& name)
        {
          auto brush = std::make_shared<script_brush>(this, name);
          this->get_scripts().push_back(brush);
          return brush;
        });

      set_function("require", [this](std::string const& name) {
        return this->require(name);
      });

      register_functions(this);

      boost::filesystem::recursive_directory_iterator end;

      if(!boost::filesystem::exists("scripts"))
      {
          _tool->addLog("[error]: 'scripts' directory does not exist");
          set_tool_error(_tool);
          return;
      }

      if(!boost::filesystem::is_directory("scripts"))
      {
          _tool->addLog("[error]: 'scripts' is not a directory");
          set_tool_error(_tool);
          return;
      }

      unsigned int error_count = 0;
      unsigned int file_count = 0;

      for (boost::filesystem::recursive_directory_iterator dir("scripts"); dir != end; ++dir)
      {
        std::string file = dir->path().string();
        if (!boost::ends_with(file, ".lua") || boost::ends_with(file, ".spec.lua"))
        {
          continue;
        }

        ++file_count;

        try
        {
          execute_file(dir->path().string());
        }
        catch (std::exception e)
        {
          ++error_count;
          std::string what = e.what();
          if (what.size() == 0)
          {
            what = std::string("Generic error in file during initialization (check function names and arguments)");
          }
          _tool->addLog("[error]: " + what + " (in file "+file+")");
          set_tool_error(_tool);
        }
      }

      if(file_count == 0)
      {
        _tool->addLog("[error]: no script files found (check your 'scripts' directory)");
        set_tool_error(_tool);
        ++error_count;
      }

      if(error_count == 0)
      {
        _tool->setStyleSheet("");
      }
    }

    scripting_tool * script_context::tool()
    {
      return _tool;
    }

    std::string script_context::get_selected_name()
    {
      if(_selected == -1)
      {
        return "";
      }
      return _scripts[_selected]->get_name();
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
      for(auto& val: this->_file_stack)
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

    World * script_context::world()
    {
      return _tool->get_view()->_world.get();
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
      _file_stack.push_back(mod);
      sol::protected_function_result res = script_file(file);
      _modules[mod] = res.get_type() == sol::type::table 
        ? res.get<sol::table>()
        : create_table();
      _file_stack.pop_back();
    }
  } // namespace scripting
} // namespace noggit