// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QDialog>

#include <functional>

class World;

namespace noggit
{
  namespace ui
  {
    class uid_fix_window : public QDialog
    {
    public:
      uid_fix_window (std::function<void()> after_fix, World*);
    };
  }
}
