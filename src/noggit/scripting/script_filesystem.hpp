// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <boost/filesystem.hpp>

#include <string>

namespace noggit
{
  namespace scripting
  {
    namespace fs = boost::filesystem;

    struct script_file_wrapper
    {
      fs::recursive_directory_iterator _dir;
      fs::recursive_directory_iterator _end;
    };

    struct script_file_iterator
    {
      script_file_iterator(fs::recursive_directory_iterator dir, fs::recursive_directory_iterator end);
      script_file_iterator() {}
      script_file_wrapper *_wrapper;
      bool _started = false;
    };

    bool file_itr_next(script_file_iterator &thiz);
    const char *file_itr_get(script_file_iterator &thiz);

    void write_file(const char *path, const char *content);
    void append_file(const char *, const char *content);
    script_file_iterator read_directory(const char *directory);
    const char *read_file(const char *path);
    bool path_exists(const char *path);
  } // namespace scripting
} // namespace noggit