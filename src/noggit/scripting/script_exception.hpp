// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <exception>
#include <string>

namespace noggit
{
  namespace scripting
  {
    class script_exception : public std::exception
    {
    public:
      script_exception(char const* msg) :
        std::exception(msg) {}
      script_exception(std::string msg) :
        std::exception(msg.c_str()) {}
    };
  } // namespace scripting
} // namespace noggit
