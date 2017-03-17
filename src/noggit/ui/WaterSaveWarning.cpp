// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/WaterSaveWarning.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>

namespace noggit
{
  namespace ui
  {
    water_save_warning::water_save_warning()
      : QWidget (nullptr)
    {
      setWindowFlags (Qt::Popup | Qt::WindowStaysOnTopHint);
      auto layout (new QFormLayout (this));

      layout->addRow (new QLabel ("Interface\\ICONS\\Ability_Creature_Poison_06.blp"));
      layout->addRow (new QLabel ("Old style water! Noggit will not"));
      layout->addRow (new QLabel ("save some water on this ADT!"));
    }
  }
}
