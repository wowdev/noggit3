// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#pragma once

#include <exception>

namespace noggit
{
    namespace scripting
    {
        class script_exception : public std::exception
        {
        public:
            script_exception(const char *msg) : std::exception(msg) {}
            script_exception(std::string msg) : std::exception(msg.c_str()) {}
        };
    } // namespace scripting
} // namespace noggit