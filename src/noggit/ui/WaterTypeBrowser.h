// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QListWidget>

class UIWater;

namespace ui
{
  class water_type_browser : public QListWidget
  {
  public:
    water_type_browser(UIWater* ui_water);
    void toggle();
  private:
    UIWater* _ui_water;
  };
}
