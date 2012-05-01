// help_widget.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef UI_HELP_WIDGET_H
#define UI_HELP_WIDGET_H

#include <QTextBrowser>

namespace noggit
{
  namespace ui
  {
    class help_widget : public QTextBrowser
    {
      Q_OBJECT

    public:
      help_widget (QWidget* parent = NULL);
    };
  }
}

#endif
