// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <boost/filesystem.hpp>

#include <string>

namespace noggit
{
  namespace scripting
  {
    namespace fs = boost::filesystem;

    void write_file(char const* path, char const* content);
    void append_file(char const* , char const* content);
    std::string read_file(char const* path);
    bool path_exists(char const* path);
  } // namespace scripting
} // namespace noggit
