// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_filesystem.hpp>

#include <das/Context.fwd.hpp>

namespace fs = boost::filesystem;

namespace noggit
{
  namespace scripting
  {
    namespace
    {
      void mkdirs(char const* pathstr)
      {
        if(pathstr == nullptr)
        {
          throw script_exception("mkdirs","empty path");
        }
        auto path = fs::path(pathstr);
        auto parent_path = path.parent_path();
        if (parent_path.string().size() > 0)
        {
          fs::create_directories(path.parent_path());
        }
      }
    }

    std::string read_file(char const* path)
    {
      if(path==nullptr)
      {
        throw script_exception("read_file","empty path");
      }
      if (!fs::exists(path))
      {
        throw script_exception("read_file","no such file:" + std::string (path));
      }
      std::ifstream t(path);
      std::string str((std::istreambuf_iterator<char>(t)),
              std::istreambuf_iterator<char>());
      return str;
    }

    void write_file(char const* path, char const* input)
    {
      if(path==nullptr)
      {
        throw script_exception("write_file","empty path");
      }
      mkdirs(path);
      if(input==nullptr)
      {
        // this shouldn't be an invalid operation.
        std::ofstream(path) << "";
      }
      else
      {
        std::ofstream(path) << input;
      }
    }

    void append_file(char const* path, char const* input)
    {
      if(path==nullptr)
      {
        throw script_exception("append_file","empty path");
      }
      mkdirs(path);
      std::ofstream outfile;
      outfile.open(path, std::ios_base::app); // append instead of overwrite
      if(input==nullptr)
      {
        outfile << "";
      }
      else
      {
        outfile << input;
      }
    }

    bool path_exists(char const* path)
    {
      if(path==nullptr)
      {
        throw script_exception("path_exists","empty path");
      }
      return fs::exists(path);
    }
  } // namespace scripting
} // namespace noggit
