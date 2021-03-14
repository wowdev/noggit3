// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <daScript/daScript.h> // must be on top

#include <noggit/Log.h>
#include <noggit/scripting/script_loader.hpp>
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/scripting_tool.hpp>

#include <mutex>

static int cur_script = -1;

using noggit::scripting::get_cur_tool;

class NoggitModule;

// NEED_MODULE macros require to be called in the global namespace (they forward-declare a register function).
static void install_modules()
{
  NEED_MODULE (Module_BuiltIn);
  NEED_MODULE (Module_Math);
  NEED_MODULE (Module_Strings);
  NEED_MODULE (Module_Rtti);
  NEED_MODULE (Module_Ast);
  NEED_MODULE (Module_Debugger);
  NEED_MODULE (Module_FIO);
  NEED_MODULE (Module_Random);
  NEED_MODULE (NoggitModule);
}

#define CALL_FUNC(ctr, type)                                         \
  if (ctr)                                                           \
  {                                                                  \
    if (ctr->_on_##type)                                             \
    {                                                                \
      auto fun = ctr->_ctx->findFunction(#type);                     \
      ctr->_ctx->eval(fun, nullptr);                                 \
      ctr->_ctx->collectStringHeap(nullptr);                         \
      if (auto ex = ctr->_ctx->getException())                       \
      {                                                              \
        get_cur_tool()->addLog("[error]: " + std::to_string(*ex));   \
        get_cur_tool()->resetLogScroll();                            \
      }                                                              \
    }                                                                \
  }

struct script_container
{
  script_container(
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
  {
  }
  script_container() = default;

  std::unique_ptr<das::Context> _ctx;

  std::string _name;
  QString _display_name;

  bool _on_select = false;

  bool _on_left_click = false;
  bool _on_left_hold = false;
  bool _on_left_release = false;

  bool _on_right_click = false;
  bool _on_right_hold = false;
  bool _on_right_release = false;
};

static std::vector<script_container> containers;

static script_container *get_container()
{
  if (cur_script < 0)
  {
    return nullptr;
  }
  return &containers[cur_script];
}

static bool ends_with(std::string const &str, std::string const &suffix)
{
  return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

// used to reroute 'print' to the script window log
class NoggitContext : public das::Context
{
public:
  NoggitContext(int stackSize) : das::Context(stackSize) {}
  virtual void to_out(char const *msg) override
  {
    // if string is empty, make an empty line
    get_cur_tool()->addLog(msg != nullptr ? msg : "");
  }

  virtual void to_err(char const *msg) override
  {
    get_cur_tool()->addLog(msg != nullptr ? msg : "");
  }
};

class : public das::TextWriter
{
public:
  virtual void output() override
  {
    int newPos = tellp();
    if (newPos != pos)
    {
      das::string st(data.data() + pos, newPos - pos);
      get_cur_tool()->addLog(st);
      pos = newPos;
    }
  }

protected:
  int pos = 0;
} _noggit_printer;

namespace noggit
{
  namespace scripting
  {
    int get_selected_script()
    {
      return cur_script;
    }

    std::string selected_script_name()
    {
      return cur_script >= 0 && cur_script < containers.size()
                 ? containers[cur_script]._name
                 : "";
    }

    std::string const &get_script_name(int id)
    {
      return containers[id]._name;
    }

    QString get_script_display_name(int id)
    {
      return containers[id]._display_name;
    }

    int script_count()
    {
      return containers.size();
    }

    void send_left_click(){CALL_FUNC(get_container(), left_click)};
    void send_left_hold(){CALL_FUNC(get_container(), left_hold)};
    void send_left_release(){CALL_FUNC(get_container(), left_release)};

    void send_right_click(){CALL_FUNC(get_container(), right_click)};
    void send_right_hold(){CALL_FUNC(get_container(), right_hold)};
    void send_right_release(){CALL_FUNC(get_container(), right_release)};

    int load_scripts()
    {
      static std::once_flag modules_installed;
      std::call_once (modules_installed, install_modules);

      das::Module::Initialize();

      std::string old_module = cur_script > 0 ? get_script_name(cur_script) : "";
      cur_script = -1;
      int new_index = -1;

      containers.clear();

      das::ModuleGroup dummyLibGroup;

      boost::filesystem::recursive_directory_iterator dir("scripts"), end;

      while (dir != end)
      {
        if (boost::filesystem::is_directory(dir->path()))
          continue;
        std::string file = dir->path().string();
        ++dir;
        if (!ends_with(file, ".das") || ends_with(file, ".spec.das"))
          continue;

        auto fAccess = das::make_smart<das::FsFileAccess>();
        auto program = das::compileDaScript(file, fAccess, _noggit_printer, dummyLibGroup, false);

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

        auto ctx = std::make_unique<NoggitContext> (program->getContextStackSize());
        if (!program->simulate(*ctx, _noggit_printer))
        {
          get_cur_tool()->setStyleSheet("background-color: #f0a5a5;");
          get_cur_tool()->addLog("Script error:");
          for (auto &err : program->errors)
          {
            get_cur_tool()->addLog(reportError(err.at, err.what, err.extra, err.fixme, err.cerr));
          }
          get_cur_tool()->resetLogScroll();

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
          auto module_name = file.substr(8, file.size() - 12);
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
              get_cur_tool()->addLog("Incorrect title type in " + module_name + " (signature should be 'def title(): string')");
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
      get_cur_tool()->setStyleSheet("");

      return new_index;
    }

    void select_script(int index)
    {
      // just for safety
      save_json();

      if (index < 0 || index >= containers.size())
      {
        return;
      }

      cur_script = index;
      auto ref = &containers[index];
      try
      {
        CALL_FUNC(ref, select);
      }
      catch (std::exception const &e)
      {
        get_cur_tool()->addLog(("[error]: " + std::string(e.what())));
        get_cur_tool()->resetLogScroll();
      }
    }
  } // namespace scripting
} // namespace noggit

#include <noggit/scripting/script_loader-noggit_module.ipp>