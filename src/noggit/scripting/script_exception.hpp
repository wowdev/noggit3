// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <stdexcept>
#include <string>

namespace noggit
{
  namespace scripting
  {
    class script_exception : public std::runtime_error
    {
    public:
      script_exception(std::string const& funcName, std::string const& msg);
    };
  } // namespace scripting
} // namespace noggit
