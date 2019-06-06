// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>
#include <noggit/ui/clickable_label.hpp>

#include <QtWidgets/QWidget>

#include <string>


class QMouseEvent;
class QDropEvent;
class QDragEnterEvent;

namespace noggit
{
  namespace ui
  {
    class current_texture : public clickable_label
    {
    private:
      std::string _filename;
      bool _need_update;

      virtual void resizeEvent (QResizeEvent*) override
      {
        update_texture_if_needed();
      }
      void update_texture_if_needed();

      virtual int heightForWidth (int) const override;

      QSize sizeHint() const override;

      QPoint _start_pos;

    public:
      current_texture();
      void set_texture (std::string const& texture);

      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

      void dragEnterEvent(QDragEnterEvent* event) override;
      void dropEvent(QDropEvent* event) override;

    };
  }
}
