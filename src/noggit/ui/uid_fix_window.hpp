// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>
#include <math/vector_3d.hpp>

class Menu;

namespace ui
{
  class uid_fix_window : public UIWindow
  {
  private:
    static const int winWidth = 320;
    static const int winHeight = 185;

    Menu* _menuLink;
    math::vector_3d _pos;

  public:
    uid_fix_window(Menu* menu);

    void enterAt(math::vector_3d const& pos);

    void fixAllTiles();
    void getMaxUID();
  };
}

