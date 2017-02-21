// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>

#include <QtWidgets/QDialog>

class Menu;

namespace ui
{
  class uid_fix_window : public QDialog
  {
  private:
    Menu* _menuLink;
    math::vector_3d _pos;

  public:
    uid_fix_window(Menu* menu);

    void enterAt(math::vector_3d const& pos);
  };
}
