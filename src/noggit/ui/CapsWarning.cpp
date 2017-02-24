// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CapsWarning.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>

namespace ui
{
  caps_warning::caps_warning()
    : QWidget (nullptr)
  {
    setWindowFlags (Qt::Popup| Qt::WindowStaysOnTopHint);
    auto layout (new QFormLayout (this));

    layout->addRow (new QLabel ("Interface\\ICONS\\INV_Sigil_Thorim.blp"));
    layout->addRow (new QLabel ("Caps lock in on!"));
    layout->addRow (new QLabel ("Turn it off to work with noggit."));
  }
}
