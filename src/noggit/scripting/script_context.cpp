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
    script_context::script_context(scripting_tool * tool)
    : _tool(tool)
    {
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
      sol::protected_function_result res = script_file(file);
      _modules[mod] = res.get_type() == sol::type::table 
        ? res.get<sol::table>()
        : create_table();
      file_stack.pop_back();
    }
  } // namespace scripting
} // namespace noggit