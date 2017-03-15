// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QPushButton>

namespace noggit
{
  namespace ui
  {
    class pushbutton : public QPushButton
    {
    public:
      template<typename Func>
        pushbutton ( QString label
                   , Func&& function
                   , QWidget* parent = nullptr
                   )
        : QPushButton (label, parent)
      {
        connect (this, &QPushButton::clicked, function);
      }
    };
  }
}
