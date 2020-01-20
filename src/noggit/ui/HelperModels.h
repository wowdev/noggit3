// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>

namespace noggit
{
  namespace ui
  {
    class object_editor;

    class helper_models : public QWidget
    {
    public:
      helper_models (object_editor* object_editor);
    };
  }
}
