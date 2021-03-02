// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <cstddef>
#include <string>

namespace noggit
{
  namespace scripting
  {
    const char* script_malloc_string(std::string const& str);
    void* script_malloc (std::size_t size);
    void* script_calloc (std::size_t size);
    void script_free_all();
    void script_free(void* ptr);
  } // namespace scripting
} // namespace noggit
