// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/cursor_selector.h>

#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QSlider>
#include <QSettings>
#include <QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    void cursor_selector::set_cursor_type (int value)
    {
      _settings->setValue ("cursor/type", value);
      _settings->sync();
    }

    void cursor_selector::set_red_color (int value)
    {
      _settings->setValue ("cursor/red", value / 255.0);
      _settings->sync();
    }

    void cursor_selector::set_green_color (int value)
    {
      _settings->setValue ("cursor/green", value / 255.0);
      _settings->sync();
    }

    void cursor_selector::set_blue_color (int value)
    {
      _settings->setValue ("cursor/blue", value / 255.0);
      _settings->sync();
    }

    void cursor_selector::set_alpha (int value)
    {
      _settings->setValue ("cursor/alpha", value / 255.0);
      _settings->sync();
    }

    cursor_selector::cursor_selector (QWidget* parent)
    : QWidget (parent)
    , _settings (new QSettings (this))
    {
      setWindowTitle (tr ("Cursor options"));

      QButtonGroup* cursor_type_group (new QButtonGroup (this));

      QRadioButton* disk_button (new QRadioButton (tr ("Disk"), this));
      QRadioButton* sphere_button (new QRadioButton (tr ("Sphere"), this));
      QRadioButton* triangle_button (new QRadioButton (tr ("Triangle"), this));
      QRadioButton* circle_button (new QRadioButton (tr ("Circle"), this));
      QRadioButton* none_button (new QRadioButton (tr ("None"), this));

      cursor_type_group->addButton (disk_button, cursor_type::disk);
      cursor_type_group->addButton (sphere_button, cursor_type::sphere);
      cursor_type_group->addButton (triangle_button, cursor_type::triangle);
      cursor_type_group->addButton (circle_button, cursor_type::circle);
      cursor_type_group->addButton (none_button, cursor_type::none);

      cursor_type_group->button (_settings->value ("cursor/type", cursor_type::circle).toInt())->click();

      connect (cursor_type_group, SIGNAL (buttonClicked (int)), SLOT (set_cursor_type (int)));

      QLabel* red_label (new QLabel (tr ("Red"), this));
      QSlider* red_slider (new QSlider (Qt::Horizontal, this));
      QLabel* green_label (new QLabel (tr ("Green"), this));
      QSlider* green_slider (new QSlider (Qt::Horizontal, this));
      QLabel* blue_label (new QLabel (tr ("Blue"), this));
      QSlider* blue_slider (new QSlider (Qt::Horizontal, this));
      QLabel* alpha_label (new QLabel (tr ("Alpha"), this));
      QSlider* alpha_slider (new QSlider (Qt::Horizontal, this));

      red_slider->setMinimum (0);
      red_slider->setMaximum (255);
      red_slider->setValue (_settings->value ("cursor/red", 1.0f).toFloat() * 255);
      green_slider->setMinimum (0);
      green_slider->setMaximum (255);
      green_slider->setValue (_settings->value ("cursor/green", 1.0f).toFloat() * 255);
      blue_slider->setMinimum (0);
      blue_slider->setMaximum (255);
      blue_slider->setValue (_settings->value ("cursor/blue", 1.0f).toFloat() * 255);
      alpha_slider->setMinimum (0);
      alpha_slider->setMaximum (255);
      alpha_slider->setValue (_settings->value ("cursor/alpha", 1.0f).toFloat() * 255);

      red_label->setBuddy (red_slider);
      green_label->setBuddy (green_slider);
      blue_label->setBuddy (blue_slider);
      alpha_label->setBuddy (alpha_slider);

      connect (red_slider, SIGNAL (valueChanged (int)), SLOT (set_red_color (int)));
      connect (green_slider, SIGNAL (valueChanged (int)), SLOT (set_green_color (int)));
      connect (blue_slider, SIGNAL (valueChanged (int)), SLOT (set_blue_color (int)));
      connect (alpha_slider, SIGNAL (valueChanged (int)), SLOT (set_alpha (int)));

      QVBoxLayout* widget_layout (new QVBoxLayout (this));
      widget_layout->addWidget (disk_button);
      widget_layout->addWidget (sphere_button);
      widget_layout->addWidget (triangle_button);
      widget_layout->addWidget (circle_button);
      widget_layout->addWidget (none_button);

      widget_layout->addWidget (red_label);
      widget_layout->addWidget (red_slider);
      widget_layout->addWidget (green_label);
      widget_layout->addWidget (green_slider);
      widget_layout->addWidget (blue_label);
      widget_layout->addWidget (blue_slider);
      widget_layout->addWidget (alpha_label);
      widget_layout->addWidget (alpha_slider);
    }
  }
}
