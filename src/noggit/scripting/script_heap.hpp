// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

namespace noggit
{
  namespace scripting
  {
    void *script_malloc(size_t size);
    void *script_calloc(size_t size);
    void script_free_all();
    void script_free(void *ptr);
  } // namespace scripting
} // namespace noggit