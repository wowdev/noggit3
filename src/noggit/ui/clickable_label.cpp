// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/clickable_label.hpp>

namespace noggit
{
  namespace ui
  {
    void clickable_label::mouseReleaseEvent (QMouseEvent* event)
    {
      emit clicked();
    }
  }
}
