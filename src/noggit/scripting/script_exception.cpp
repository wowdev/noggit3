// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <string>
#include <exception>

namespace noggit
{
  namespace scripting
  {
    script_exception::script_exception(std::string const& funcName, std::string const& msg)
    : std::runtime_error(msg+" (in function "+funcName+")")
    {
        // TEMP: remove when exceptions are working
        if(msg.find("C++ exception") != 0)
        {
          set_cur_exception(std::string(what()));
        }
    }
  } // namespace scripting
} // namespace noggit
