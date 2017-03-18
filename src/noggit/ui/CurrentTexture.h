// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>
#include <noggit/ui/clickable_label.hpp>

#include <QtWidgets/QWidget>

#include <string>

namespace noggit
{
  namespace ui
  {
    class current_texture : public clickable_label
    {
    private:
      std::string _filename;

      virtual void resizeEvent (QResizeEvent*) override
      {
        update_texture();
      }
      void update_texture();

      virtual int heightForWidth (int) const override;

    public:
      current_texture();
      void set_texture (std::string const& texture);
    };
  }
}
