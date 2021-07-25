// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/errorHandling.h>
#include <util/exception_to_string.hpp>

namespace util
{
  namespace
  {
    std::string exception_to_string_impl
      (std::exception_ptr const& ep, std::size_t indentation, bool is_first)
    {
      auto const prefix {(is_first ? "" : "\n") + std::string (indentation, ' ')};

      try
      {
        std::rethrow_exception (ep);
      }
      catch (std::exception const& e)
      {
        std::string res {prefix + e.what()};

        try
        {
          std::rethrow_if_nested (e);
        }
        catch (...)
        {
          res += exception_to_string_impl
            (std::current_exception(), indentation + 1, false);
        }

        return res;
      }
      catch (...)
      {
        return prefix + "<unknown exception type>";
      }

      return prefix + "<no exception>";
    }
  }

  std::string exception_to_string (std::exception_ptr ptr)
  {
    noggit::printStacktrace();
    return exception_to_string_impl (ptr, 0, true);
  }
}
