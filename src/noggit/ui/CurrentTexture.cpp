// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/texture_swapper.hpp>

#include <noggit/tool_enums.hpp>

#include <QtWidgets/QGridLayout>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtGui/QDrag>
#include <QMimeData>

namespace noggit
{
  namespace ui
  {
    current_texture::current_texture(bool accept_drop, QWidget* parent)
      : clickable_label (parent)
      , _filename("tileset\\generic\\black.blp")
      , _need_update(true)
    {
      QSizePolicy policy (QSizePolicy::Maximum, QSizePolicy::Maximum);
      setSizePolicy (policy);
      setMinimumSize(128, 128);
      setAcceptDrops(accept_drop);

      update_texture_if_needed();
    }

    QSize current_texture::sizeHint() const
    {
      return QSize(128, 128);
    }

    int current_texture::heightForWidth (int width) const
    {
      return width;
    }

    void current_texture::set_texture (std::string const& texture)
    {
      _filename = texture;
      _need_update = true;
      update_texture_if_needed();
    }

    void current_texture::update_texture_if_needed()
    {
      if (!_need_update)
      {
        return;
      }

      _need_update = false;

      show();
      setPixmap (render_blp_to_pixmap (_filename, width(), height()));
      setToolTip(QString::fromStdString(_filename));
    }

    void current_texture::mousePressEvent(QMouseEvent* event)
    {
      if (event->button() == Qt::LeftButton)
      {
        _start_pos = event->pos();
      }

      clickable_label::mousePressEvent(event);
    }

    void current_texture::mouseMoveEvent(QMouseEvent* event)
    {
      clickable_label::mouseMoveEvent(event);

      if (!(event->buttons() & Qt::LeftButton))
      {
        return;
      }

      int drag_dist = (event->pos() - _start_pos).manhattanLength();

      if (drag_dist < QApplication::startDragDistance())
      {
        return;
      }

      QMimeData* mimeData = new QMimeData;
      mimeData->setText(QString(_filename.c_str()));

      QDrag* drag = new QDrag(this);
      drag->setMimeData(mimeData);
      drag->setPixmap(*pixmap());
      drag->exec();
    }

    void current_texture::dragEnterEvent(QDragEnterEvent* event)
    {
      if (event->mimeData()->hasText())
      {
        event->accept();
      }
    }

    void current_texture::dropEvent(QDropEvent* event)
    {
      std::string filename = event->mimeData()->text().toStdString();

      set_texture(filename);
      emit texture_dropped(filename);
    }
  }
}
