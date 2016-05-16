// help_widget.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#pragma once

#include <QTextBrowser>

namespace noggit
{
  namespace ui
  {
    class help_widget : public QTextBrowser
    {
      Q_OBJECT

    public:
      help_widget (QWidget* parent = nullptr);
    };
  }
}
