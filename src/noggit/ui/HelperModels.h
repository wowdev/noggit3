// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>

class MapView;

namespace noggit
{
  namespace ui
  {
    class helper_models : public QWidget
    {
    public:
      helper_models (MapView *mapview);
    };
  }
}
