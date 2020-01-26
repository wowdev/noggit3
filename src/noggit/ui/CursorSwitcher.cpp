// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CursorSwitcher.h>
#include <noggit/tool_enums.hpp>
#include <util/qt/overload.hpp>

#include <qt-color-widgets/color_selector.hpp>

#include <QtCore/QSettings>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    cursor_switcher::cursor_switcher(QWidget* parent, math::vector_4d& color, noggit::unsigned_int_property& cursor_type)
      : widget (parent)
    {
      setWindowTitle("Cursor Options");
      setWindowFlags(Qt::Tool);
      new QVBoxLayout (this);

      auto butt_disk (new QRadioButton ("Disk", this));
      auto butt_sphere (new QRadioButton ("Sphere", this));
      auto butt_terrain_disk (new QRadioButton ("Terrain Disk", this));
      auto butt_none (new QRadioButton ("None", this));

      this->layout()->addWidget (butt_disk);
      this->layout()->addWidget (butt_sphere);
      this->layout()->addWidget (butt_terrain_disk);
      this->layout()->addWidget (butt_none);

      auto group (new QButtonGroup (this));

      group->addButton (butt_disk, static_cast<int>(cursor_mode::disk));
      group->addButton (butt_sphere, static_cast<int>(cursor_mode::sphere));
      group->addButton (butt_terrain_disk, static_cast<int>(cursor_mode::terrain));
      group->addButton (butt_none, static_cast<int>(cursor_mode::none));

      group->button (cursor_type.get())->setChecked (true);

      connect ( group
              , qOverload<int> (&QButtonGroup::buttonClicked)
               , [&] (int id)
                 {
                   QSignalBlocker const blocker(&cursor_type);
                   cursor_type.set(id);
                   QSettings settings;
                   settings.setValue ("cursor/default_type", id);
                 }
              );

      connect ( &cursor_type
              , &noggit::unsigned_int_property::changed
              , [group] (unsigned int id)
                {
                  QSignalBlocker const blocker(group);
                  group->button(id)->setChecked (true);
                }
              );

      auto color_picker (new color_widgets::ColorSelector (this));

      color_picker->setDisplayMode (color_widgets::ColorSelector::AllAlpha);

      color_picker->setColor (QColor::fromRgbF (color.x, color.y, color.z, color.w));

      connect ( color_picker, &color_widgets::ColorSelector::colorChanged
              , [&] (QColor new_color)
                {
                  color.x = new_color.redF();
                  color.y = new_color.greenF();
                  color.z = new_color.blueF();
                  color.w = new_color.alphaF();
                  QSettings settings;
                  settings.setValue ("cursor/color/r", color.x);
                  settings.setValue ("cursor/color/g", color.y);
                  settings.setValue ("cursor/color/b", color.z);
                  settings.setValue ("cursor/color/a", color.w);
                  
                }
              );


      this->layout()->addWidget (color_picker);
    }
  }
}
