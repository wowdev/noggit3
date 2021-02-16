#pragma once

#include <exception>

namespace noggit {
    namespace scripting {
        class script_exception : public std::exception {
        public:
            script_exception(const char* msg) : std::exception(msg){}
            script_exception(std::string msg): std::exception(msg.c_str()){}
        };
    }
}