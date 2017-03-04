// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>
#include <noggit/ui/clickable_label.hpp>

#include <QtWidgets/QWidget>

#include <string>

class UIFrame;

namespace noggit
{
  namespace ui
  {
    class current_texture : public QWidget
    {
    private:
      clickable_label* _texture;
      std::string _filename;

      virtual void resizeEvent (QResizeEvent* event) override
      {
        update();
      }
      void update();

    public:
      current_texture (UIFrame* texture_palette);
      void set_texture (std::string const& texture);
    };
  }
}
