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
    current_texture::current_texture(texture_swapper* texture_switcher)
      : clickable_label (nullptr)
      , _filename("tileset\\generic\\black.blp")
      , _need_update(true)
    {
      QSizePolicy policy (QSizePolicy::Maximum, QSizePolicy::Maximum);
      setSizePolicy (policy);
      setMinimumSize(128, 128);
      setAcceptDrops(true);

      update_texture_if_needed();

      _texture_switcher = texture_switcher;
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
        _start_pos = event->pos();

      clickable_label::mousePressEvent(event);
    }

    void current_texture::mouseMoveEvent(QMouseEvent* event)
    {
      clickable_label::mouseMoveEvent(event);

      if (!(event->buttons() & Qt::LeftButton))
        return;
      if ((event->pos() - _start_pos).manhattanLength()
        < QApplication::startDragDistance())
        return;

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
        event->accept();
    }

    void current_texture::dropEvent(QDropEvent* event)
    {

      std::string filename = event->mimeData()->text().toStdString();

      switch (_drop_behavior)
      {
        case CurrentTextureDropBehavior::current_texture:
        {
          set_texture(filename);
          noggit::ui::selected_texture::set(filename);
          event->accept();
          break;
        }
        case CurrentTextureDropBehavior::texture_swapper:
        {
          _texture_switcher->set_texture(filename);
          break;
        }
        case CurrentTextureDropBehavior::none:
          return;
      }

    }

    void current_texture::set_drop_behavior(int behavior)
    {
      _drop_behavior = behavior;
    }

  }
}
