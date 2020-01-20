// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>
#include <noggit/ui/font_noggit.hpp>

class QFormLayout;

namespace noggit
{
  namespace ui
  {
    class help : public widget
    {
    public:
      help(QWidget* parent = nullptr);

    private:
      inline void generate_hotkey_row(std::initializer_list<font_noggit::icons>&& hotkeys, const char* description, QFormLayout* layout);
    };
  }
}
