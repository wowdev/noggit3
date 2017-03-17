// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QWidget>

class MapView;

namespace noggit
{
  namespace ui
  {
    class model_import : public QWidget
    {
    private:
      QListWidget* _list;
      QLineEdit* _textBox;

    public:
      model_import (MapView *mapview);
      void buildModelList();
    };
  }
}
