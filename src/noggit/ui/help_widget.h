#ifndef UI_HELP_WIDGET_H
#define UI_HELP_WIDGET_H

#include <QTextBrowser>

namespace ui
{
  class help_widget : public QTextBrowser
  {
  public:
    help_widget (QWidget* parent = NULL);
  };
}

#endif
