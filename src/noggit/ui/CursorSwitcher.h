// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>

#include <math/vector_4d.hpp>

namespace noggit
{
  namespace ui
  {
    class cursor_switcher : public widget
    {
    public:
      cursor_switcher(math::vector_4d& color, int& cursor_type);
    };
  }
}
