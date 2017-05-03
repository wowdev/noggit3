// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/unsigned_int_property.hpp>
#include <noggit/ui/widget.hpp>

#include <math/vector_4d.hpp>

namespace noggit
{
  namespace ui
  {
    class cursor_switcher : public widget
    {
    public:
      cursor_switcher(QWidget* parent, math::vector_4d& color, noggit::unsigned_int_property& cursor_type);
    };
  }
}
