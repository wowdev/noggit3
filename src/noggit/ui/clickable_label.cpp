// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/clickable_label.hpp>

namespace ui
{
  clickable_label::clickable_label (QWidget* parent) 
    : QLabel(parent) 
  {
  } 

  void clickable_label::mouseReleaseEvent(QMouseEvent* event) 
  {
    emit clicked();
  }
}
