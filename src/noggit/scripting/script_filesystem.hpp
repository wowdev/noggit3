// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <boost/filesystem.hpp>

#include <string>

namespace noggit
{
  namespace scripting
  {
    class script_context;
    class scripting_tool;
    namespace fs = boost::filesystem;
    boost::filesystem::path get_writable_path(std::string const& caller, script_context * state, std::string const& path);
    void write_file(script_context * ctx, std::string const& path, std::string const& content);
    void append_file(script_context * ctx, std::string const& path, std::string const& content);
    std::string read_file(std::string const& path);
    bool path_exists(std::string const& path);
    void register_filesystem(script_context * state);
    void mkdirs(std::string const& path);
  } // namespace scripting
} // namespace noggit
