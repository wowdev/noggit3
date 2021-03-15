// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Log.h>
#include <noggit/scripting/script_loader.hpp>
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/scripting_tool.hpp>

#include <das/AnnotationArgument.hpp>
#include <das/cast.hpp>
#include <das/compileDaScript.hpp>
#include <das/FsFileAccess.hpp>
#include <das/make_smart.hpp>
#include <das/Module.hpp>
#include <das/ModuleGroup.hpp>
#include <das/need_module.hpp>
#include <das/TextPrinter.hpp>
#include <das/Type.hpp>
#include <das/verifyCall.hpp>

#include <mutex>

DAS_FORWARD_DECLARE_MODULE (Module_BuiltIn);
DAS_FORWARD_DECLARE_MODULE (Module_Math);
DAS_FORWARD_DECLARE_MODULE (Module_Strings);
DAS_FORWARD_DECLARE_MODULE (Module_Rtti);
DAS_FORWARD_DECLARE_MODULE (Module_Ast);
DAS_FORWARD_DECLARE_MODULE (Module_Debugger);
DAS_FORWARD_DECLARE_MODULE (Module_FIO);
DAS_FORWARD_DECLARE_MODULE (Module_Random);
DAS_FORWARD_DECLARE_MODULE (NoggitModule);

class NoggitModule;

#define CALL_FUNC(ctr, tool, type)                                   \
  if (ctr)                                                           \
  {                                                                  \
    if (ctr->_on_##type)                                             \
    {                                                                \
      auto fun = ctr->_ctx->findFunction(#type);                     \
      ctr->_ctx->eval(fun, nullptr);                                 \
      ctr->_ctx->collectStringHeap(nullptr);                         \
      if (auto ex = ctr->_ctx->getException())                       \
      {                                                              \
        tool->addLog("[error]: " + std::to_string(*ex));             \
        tool->resetLogScroll();                                      \
      }                                                              \
    }                                                                \
  }

namespace noggit
{
  namespace scripting
  {
    Loader::Context::Context(int stackSize, noggit::scripting::scripting_tool* tool)
      : das::Context(stackSize)
      , _tool (tool)
    {}

    void Loader::Context::to_out(char const *msg)
    {
      // if string is empty, make an empty line
      _tool->addLog(msg != nullptr ? msg : "");
    }
    void Loader::Context::to_err(char const *msg)
    {
      _tool->addLog(msg != nullptr ? msg : "");
    }

    Loader::script_container::script_container(
      std::string name,
      QString display_name,
      std::unique_ptr<das::Context> ctx,
      bool select,
      bool left_click,
      bool left_hold,
      bool left_release,
      bool right_click,
      bool right_hold,
      bool right_release) : _ctx(std::move (ctx)),
                            _name(name),
                            _display_name(display_name),
                            _on_select(select),
                            _on_left_click(left_click),
                            _on_left_hold(left_hold),
                            _on_left_release(left_release),
                            _on_right_click(right_click),
                            _on_right_hold(right_hold),
                            _on_right_release(right_release)
    {}

    int Loader::get_selected_script() const
    {
      return cur_script;
    }

    std::string Loader::selected_script_name() const
    {
      return cur_script >= 0 && cur_script < containers.size()
                 ? containers[cur_script]._name
                 : "";
    }

    std::string const& Loader::get_script_name(int id) const
    {
      return containers[id]._name;
    }

    QString Loader::get_script_display_name(int id) const
    {
      return containers[id]._display_name;
    }

    int Loader::script_count() const
    {
      return containers.size();
    }

#define CURRENT_CONTEXT_CALL_FUNC(tool, type)                             \
  do                                                                      \
  {                                                                       \
    auto container (cur_script < 0 ? nullptr : &containers[cur_script]);  \
    CALL_FUNC (container, tool, type);                                    \
  } while (false)

    void Loader::send_left_click (scripting_tool* tool)
    {
      CURRENT_CONTEXT_CALL_FUNC (tool, left_click);
    }
    void Loader::send_left_hold (scripting_tool* tool)
    {
      CURRENT_CONTEXT_CALL_FUNC (tool, left_hold);
    }
    void Loader::send_left_release (scripting_tool* tool)
    {
      CURRENT_CONTEXT_CALL_FUNC (tool, left_release);
    }

    void Loader::send_right_click (scripting_tool* tool)
    {
      CURRENT_CONTEXT_CALL_FUNC (tool, right_click);
    }
    void Loader::send_right_hold (scripting_tool* tool)
    {
      CURRENT_CONTEXT_CALL_FUNC (tool, right_hold);
    }
    void Loader::send_right_release (scripting_tool* tool)
    {
      CURRENT_CONTEXT_CALL_FUNC (tool, right_release);
    }

    int Loader::load_scripts (scripting_tool* tool)
    {
      static std::once_flag modules_installed;
      std::call_once (modules_installed, [] {
        DAS_NEED_MODULE (Module_BuiltIn);
        DAS_NEED_MODULE (Module_Math);
        DAS_NEED_MODULE (Module_Strings);
        DAS_NEED_MODULE (Module_Rtti);
        DAS_NEED_MODULE (Module_Ast);
        DAS_NEED_MODULE (Module_Debugger);
        DAS_NEED_MODULE (Module_FIO);
        DAS_NEED_MODULE (Module_Random);
        DAS_NEED_MODULE (NoggitModule);
      });

      das::Module::Initialize();

      std::string old_module = cur_script > 0 ? get_script_name(cur_script) : "";
      cur_script = -1;
      int new_index = -1;

      containers.clear();

      das::ModuleGroup dummyLibGroup;

      for (boost::filesystem::recursive_directory_iterator dir ("scripts"), end; dir != end; ++dir)
      {
        if (boost::filesystem::is_directory(dir->path()))
          continue;
        if (dir->path().extension() != ".das")
          continue;
        if (dir->path().stem() == "noggit.spec")
          continue;

        struct Printer : public das::TextPrinter
        {
          Printer (scripting_tool* tool)
            : _tool (tool)
          {}

          virtual void output() override
          {
            int newPos = tellp();
            if (newPos != pos)
            {
              std::string st(data.data() + pos, newPos - pos);
              _tool->addLog(st);
              pos = newPos;
            }
          }

          noggit::scripting::scripting_tool* _tool;
        } noggit_printer = {tool};

        auto fAccess = das::make_smart<das::FsFileAccess>();
        auto program = das::compileDaScript(dir->path().string(), fAccess, noggit_printer, dummyLibGroup, false);

        if (!program->options.find("persistent_heap", das::Type::tBool))
        {
          program->options.push_back(das::AnnotationArgument("persistent_heap", true));
        }

        if (!program->options.find("persistent_string_heap", das::Type::tBool))
        {
          program->options.push_back(das::AnnotationArgument("persistent_string_heap", true));
        }

        if (!program->options.find("intern_strings", das::Type::tBool))
        {
          program->options.push_back(das::AnnotationArgument("intern_strings", false));
        }

        auto ctx = std::make_unique<Loader::Context> (program->getContextStackSize(), tool);
        if (!program->simulate(*ctx, noggit_printer))
        {
          tool->setStyleSheet("background-color: #f0a5a5;");
          tool->addLog("Script error:");
          for (auto &err : program->errors)
          {
            tool->addLog(reportError(err.at, err.what, err.extra, err.fixme, err.cerr));
          }
          tool->resetLogScroll();

          containers.clear();
          return -1;
        }

        bool is_any = false;

#define CHECK_FUNC(ctx, type)                                       \
  auto on_##type = ctx->findFunction(#type);                        \
  auto is_##type = false;                                           \
  if (on_##type)                                                    \
  {                                                                 \
    if (das::verifyCall<void>(on_##type->debugInfo, dummyLibGroup)) \
    {                                                               \
      is_##type = true;                                             \
      is_any = true;                                                \
    }                                                               \
  }

        CHECK_FUNC(ctx, select);
        CHECK_FUNC(ctx, left_click);
        CHECK_FUNC(ctx, left_hold);
        CHECK_FUNC(ctx, left_release);

        CHECK_FUNC(ctx, right_click);
        CHECK_FUNC(ctx, right_hold);
        CHECK_FUNC(ctx, right_release);

        if (is_any)
        {
          auto module_name = dir->path().stem().string();
          if (module_name == old_module)
          {
            new_index = containers.size();
          }

          QString display_name = QString::fromStdString (module_name);
          auto title_fun = ctx->findFunction("title");
          if (title_fun)
          {
            if (!das::verifyCall<char const *>(title_fun->debugInfo, dummyLibGroup))
            {
              tool->addLog("Incorrect title type in " + module_name + " (signature should be 'def title(): string')");
            }
            else
            {
              auto result = das::cast<char const *>::to(ctx->eval(title_fun));
              if (result)
              {
                display_name = result;
              }
            }
          }

          containers.emplace_back(
              module_name,
              display_name,
              std::move (ctx),
              is_select,
              is_left_click,
              is_left_hold,
              is_left_release,
              is_right_click,
              is_right_hold,
              is_right_release);
        }
      }

      if (new_index == -1 && containers.size() > 0)
      {
        new_index = 0;
      }

      // remove error color on successful compile
      tool->setStyleSheet("");

      return new_index;
    }

    void Loader::select_script (int index, scripting_tool* tool)
    {
      // just for safety
      tool->save_json();

      if (index < 0 || index >= containers.size())
      {
        return;
      }

      cur_script = index;
      auto ref = &containers[index];
      try
      {
        CALL_FUNC(ref, tool, select);
      }
      catch (std::exception const &e)
      {
        tool->addLog(("[error]: " + std::string(e.what())));
        tool->resetLogScroll();
      }
    }
  } // namespace scripting
} // namespace noggit

#include <noggit/scripting/script_loader-noggit_module.ipp>
