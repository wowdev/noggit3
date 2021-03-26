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
    namespace
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
      bool get_write_permission(script_context * state, std::string const& path)
      {
        if (state->tool()->get_noggit_settings()->value("allow_scripts_write_any_file", false).toBool())
        {
          return true;
        }

        boost::filesystem::path boost_path = boost::filesystem::weakly_canonical(boost::filesystem::path(path));
        if (allowed_files.find(boost_path) != allowed_files.end())
        {
          return true;
        }
        QMessageBox prompt;
        prompt.setText(std::string("A script wants to write to the file "+boost_path.string()).c_str());
        prompt.setInformativeText(std::string("Do you want to allow the script to write to "+boost_path.string()+"?").c_str());
        prompt.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
        prompt.setDefaultButton(QMessageBox::No);
        bool answer = prompt.exec() == QMessageBox::StandardButton::Yes;
        if(answer)
        {
          allowed_files.insert(boost_path);
        }
        return answer;
      }
    }

    void write_file(script_context * ctx, std::string const& path, std::string const& input)
    {
      if(!get_write_permission(ctx, path))
      {
        throw script_exception("write_file","Not allowed to write to "+path);
      }
      mkdirs(path);
      std::ofstream(path) << input;
    }

    void append_file(script_context * ctx, std::string const& path, std::string const& input)
    {
      if(!get_write_permission(ctx, path))
      {
        throw script_exception("write_file","Not allowed to write to "+path);
      }
      mkdirs(path);
      std::ofstream outfile;
      outfile.open(path, std::ios_base::app); // append instead of overwrite
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
