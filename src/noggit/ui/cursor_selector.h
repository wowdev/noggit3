#ifndef CURSOR_SELECTOR_H
#define CURSOR_SELECTOR_H

#include <QWidget>

class QSettings;

namespace noggit
{
  namespace ui
  {
    class cursor_selector : public QWidget
    {
      Q_OBJECT
    public:
      cursor_selector (QWidget* parent = NULL);

    public slots:
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

#endif
