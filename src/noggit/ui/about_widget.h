// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QWidget>

namespace noggit
{
  namespace ui
  {
    class about_widget : public QWidget
    {
      Q_OBJECT

    public:
      about_widget (QWidget* parent = nullptr);
    };
  }
}
