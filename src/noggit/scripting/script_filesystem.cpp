// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/ui/SettingsPanel.h>

#include <QtWidgets/QMessageBox>
#include <boost/filesystem.hpp>
#include <sol/sol.hpp>

#include <set>

namespace fs = boost::filesystem;

namespace noggit
{
  namespace scripting
  {
    void mkdirs(std::string const& pathstr)
    {
      auto path = fs::path(pathstr);
      auto parent_path = path.parent_path();
      if (parent_path.string().size() > 0)
      {
        fs::create_directories(path.parent_path());
      }
    }

    std::string read_file(std::string const& path)
    {
      if (!fs::exists(path))
      {
        throw script_exception("read_file","no such file:" + std::string (path));
      }
      std::ifstream t(path);
      std::string str((std::istreambuf_iterator<char>(t)),
              std::istreambuf_iterator<char>());
      return str;
    }

    namespace {
      std::set<boost::filesystem::path> allowed_files;
    }

    boost::filesystem::path get_writable_path(std::string const& caller, script_context * state, std::string const& path)
    {
      auto canonical = boost::filesystem::weakly_canonical(boost::filesystem::path(path));
      if (state->tool()->get_noggit_settings()->value("allow_scripts_write_any_file", false).toBool())
      {
        return canonical;
      }
      if (allowed_files.find(canonical) != allowed_files.end())
      {
        return canonical;
      }
      QMessageBox prompt;
      prompt.setText(std::string("A script wants to write to the file "+canonical.string()).c_str());
      prompt.setInformativeText(std::string("Do you want to allow the script to write to "+canonical.string()+"?").c_str());
      prompt.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
      prompt.setDefaultButton(QMessageBox::No);
      bool answer = prompt.exec() == QMessageBox::StandardButton::Yes;
      if(!answer)
      {
        throw script_exception(caller,"No permission to write file "+canonical.string());
      }
      return *allowed_files.emplace (canonical).first;
    }

    void write_file(script_context * ctx, std::string const& path, std::string const& input)
    {
      auto writable_path = get_writable_path("write_file",ctx,path);
      mkdirs(writable_path.string());
      std::ofstream(writable_path.string()) << input;
    }

    void append_file(script_context * ctx, std::string const& path, std::string const& input)
    {
      auto writable_path = get_writable_path("append_file",ctx,path);
      mkdirs(writable_path.string());
      std::ofstream outfile;
      outfile.open(writable_path.string(), std::ios_base::app); // append instead of overwrite
      outfile << input;
    }

    bool path_exists(std::string const& path)
    {
      return fs::exists(path);
    }

    void register_filesystem(script_context * state)
    {
      state->set_function("write_file", [state](
          std::string const& path
        , std::string const& input
        ) {
          write_file(state, path, input);
        });
      state->set_function("append_file", [state](
          std::string const& path
        , std::string const& input
        ) {
          append_file(state, path, input);
        });
      state->set_function("read_file",read_file);
      state->set_function("path_exists",path_exists);
    }
  } // namespace scripting
} // namespace noggit
