// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>

#include <QtWidgets/QLabel>

#include <string>

namespace ui
{
  class detail_infos : public noggit::ui::widget
  {
  private:
    QLabel* _info_text;

  public:
    detail_infos (float x, float y, float width, float height);
    void setText (const std::string& t);
  };
}
