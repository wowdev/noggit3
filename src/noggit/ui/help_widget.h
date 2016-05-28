// This file is part of Noggit3, licensed under GNU General Public License (version 3).

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
