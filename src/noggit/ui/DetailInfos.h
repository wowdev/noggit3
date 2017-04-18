// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>

#include <QtWidgets/QLabel>

#include <string>

namespace noggit
{
  namespace ui
  {
    class detail_infos : public widget
    {
    private:
      QLabel* _info_text;

    public:
      detail_infos(QWidget* parent);
      void setText (const std::string& t);
    };
  }
}
