// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_heap.hpp>

namespace fs = boost::filesystem;

namespace noggit
{
  namespace scripting
  {
    static void skip_dirs(script_file_iterator& itr)
    {
      while (itr._wrapper->_dir != itr._wrapper->_end &&
           boost::filesystem::is_directory(itr._wrapper->_dir->path()))
      {
        ++itr._wrapper->_dir;
      }
    }

    static void mkdirs(char const* pathstr)
    {
      auto path = fs::path(pathstr);
      auto parent_path = path.parent_path();
      if (parent_path.string().size() > 0)
      {
        fs::create_directories(path.parent_path());
      }
    }

    char const* read_file(char const* path)
    {
      if (!fs::exists(path))
      {
        throw script_exception("No such file:" + std::string (path));
      }
      std::ifstream t(path);
      std::string str((std::istreambuf_iterator<char>(t)),
              std::istreambuf_iterator<char>());
      return script_malloc_string(str);
    }

    void write_file(char const* path, char const* input)
    {
      mkdirs(path);
      std::ofstream(path) << input;
    }

    void append_file(char const* path, char const* input)
    {
      mkdirs(path);
      std::ofstream outfile;
      outfile.open(path, std::ios_base::app); // append instead of overwrite
      outfile << input;
    }

    bool path_exists(char const* path)
    {
      return fs::exists(path);
    }

    script_file_iterator read_directory(char const* path)
    {
      fs::recursive_directory_iterator
        dir(path == nullptr ? "" : path),
        end;
      return script_file_iterator(dir, end);
    }

    script_file_iterator::script_file_iterator(fs::recursive_directory_iterator dir, fs::recursive_directory_iterator end)
    {
      _wrapper = (script_file_wrapper*)script_calloc(sizeof(script_file_wrapper));
      _wrapper->_dir = dir;
      _wrapper->_end = end;
      skip_dirs(*this);
    }

    char const* file_itr_get(script_file_iterator const& itr)
    {
      return itr._wrapper->_dir->path().string().c_str();
    }

    bool file_itr_next(script_file_iterator& itr)
    {
      if (!itr._started)
      {
        itr._started = true;
        return itr._wrapper->_dir != itr._wrapper->_end;
      }

      if (itr._wrapper->_dir != itr._wrapper->_end)
      {
        ++itr._wrapper->_dir;
        skip_dirs(itr);
      }

      return itr._wrapper->_dir != itr._wrapper->_end;
    }
  } // namespace scripting
} // namespace noggit
