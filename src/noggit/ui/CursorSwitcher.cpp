// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CursorSwitcher.h>

#include <qt-color-widgets/color_selector.hpp>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

UICursorSwitcher::UICursorSwitcher(math::vector_4d& color, int& cursor_type)
  : QDockWidget ("Cursor Options", nullptr, nullptr)
{
  setFloating(true);

  QWidget *content = new QWidget(nullptr);
  new QVBoxLayout (content);

  auto butt_disk (new QRadioButton ("Disk", content));
  auto butt_sphere (new QRadioButton ("Sphere", content));
  auto butt_triangle (new QRadioButton ("Triangle", content));
  auto butt_none (new QRadioButton ("None", content));

  content->layout()->addWidget (butt_disk);
  content->layout()->addWidget (butt_sphere);
  content->layout()->addWidget (butt_triangle);
  content->layout()->addWidget (butt_none);

  auto group (new QButtonGroup (content));

  group->addButton (butt_disk, 1);
  group->addButton (butt_sphere, 2);
  group->addButton (butt_triangle, 3);
  group->addButton (butt_none, 0);

  group->button (cursor_type)->setChecked (true);

  connect ( group, static_cast<void (QButtonGroup::*) (int)> (&QButtonGroup::buttonClicked)
          , [&] (int id)
            {
              cursor_type = id;
            }
          );


  auto color_picker (new color_widgets::ColorSelector (content));

  color_picker->setDisplayMode (color_widgets::ColorSelector::AllAlpha);

  color_picker->setColor (QColor::fromRgbF (color.x, color.y, color.z, color.w));

  connect ( color_picker, &color_widgets::ColorSelector::colorChanged
          , [&] (QColor new_color)
            {
              color.x = new_color.redF();
              color.y = new_color.greenF();
              color.z = new_color.blueF();
              color.w = new_color.alphaF();
            }
          );


  content->layout()->addWidget (color_picker);
    setWidget(content);
}
