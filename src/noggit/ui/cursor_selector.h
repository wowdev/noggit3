// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QWidget>

class QSettings;

namespace noggit
{
  namespace ui
  {
    namespace cursor_type
    {
      enum cursor_type
      {
        none = 0,
        disk = 1,
        sphere = 2,
        triangle = 3,
        circle = 4,
        num_cursors,
      };
    }

    class cursor_selector : public QWidget
    {
      Q_OBJECT

    public:
      cursor_selector (QWidget* parent = nullptr);

    private slots:
      void set_cursor_type (int value);
      void set_red_color (int value);
      void set_green_color (int value);
      void set_blue_color (int value);
      void set_alpha (int value);

    private:
      QSettings* _settings;
    };
  }
}
