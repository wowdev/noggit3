// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <cstddef>
#include <string>

namespace das {
  class Context;
}

namespace noggit
{
  namespace scripting
  {
    const char* script_calloc_string(std::string const& str, das::Context* ctx);

    char* script_calloc (unsigned size, das::Context* ctx);
    void script_free_all();
    //void script_free(script_heap_ptr const& ptr, das::Context* ctx);
    void script_free_string(char* ptr, das::Context* ctx);

    // test functions
    int script_heap_read_byte(const char* str, int offset);
    void script_heap_write_byte(const char* str, int offset, int byte);
    bool overlaps(const char* str1, int length1, const char* str2, int length2);
  } // namespace scripting
} // namespace noggit
