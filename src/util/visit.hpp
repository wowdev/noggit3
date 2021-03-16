#pragma once

namespace util
{
  template <typename Variant, typename... Funs>
    auto visit (Variant& variant, Funs... funs);
}

#include <util/visit.ipp>
