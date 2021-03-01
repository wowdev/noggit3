// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <cstddef>

namespace noggit
{
  namespace scripting
  {
    void* script_malloc (std::size_t size);
    void* script_calloc (std::size_t size);
    void script_free_all();
    void script_free(void* ptr);
  } // namespace scripting
} // namespace noggit
