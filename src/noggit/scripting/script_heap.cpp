// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <daScript/daScript.h>

#include <noggit/scripting/script_heap.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <vector>
#include <algorithm>

namespace noggit
{
  namespace scripting
  {
    char* script_calloc (unsigned size, das::Context* ctx)
    {
      // Force das to store these in the "big things" part of the string heap
      // (256 should suffice, but not taking any risks)
      size = std::max(unsigned(257),size);
      auto g = ctx->stringHeap->allocate(size);
      return g;
    }

    const char* script_calloc_string(std::string const& str, das::Context* ctx)
    {
      return ctx->stringHeap->allocateString(str);
    }

    bool overlaps(const char* str1, int length1, const char* str2, int length2)
    {
      unsigned long p1 = (unsigned long)str1;
      unsigned long p2 = (unsigned long)str2;
      return p1 <= p2+length2 && p2 <= p1+length1;
    }

    int script_heap_read_byte(const char* str, int offset)
    {
      return int((unsigned char)str[offset]);
    }

    void script_heap_write_byte(const char* str, int offset, int byte)
    {
      ((unsigned char*)str)[offset] = static_cast<unsigned char>(unsigned(byte));
    }

    void script_free_all()
    {
      // TODO: Disable
    }
  } // namespace scripting
} // namespace noggit
