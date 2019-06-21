// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/TextureList.hpp>

#include <noggit/Misc.h>

#include <QtWidgets/QListView>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtGui/QDrag>
#include <QMimeData>


namespace noggit
{
  namespace ui
  {
    TextureList::TextureList(QWidget* parent)
      : QListView(parent)
    {
      setDragEnabled(true);
    }

    void TextureList::mousePressEvent(QMouseEvent* event)
    {
      if (event->button() == Qt::LeftButton)
        _start_pos = event->pos();

      QListView::mousePressEvent(event);
    }

    void TextureList::mouseMoveEvent(QMouseEvent* event)
    {
      QListView::mouseMoveEvent(event);

      if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - _start_pos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
        {
          QModelIndex index = currentIndex();
          const std::string filename = index.data(Qt::DisplayRole).toString().prepend("tileset/").toStdString();

          QMimeData* mimeData = new QMimeData;
          mimeData->setText(QString(filename.c_str()));


          QDrag* drag = new QDrag(this);
          drag->setMimeData(mimeData);
          drag->setPixmap(index.data(Qt::DecorationRole).value<QIcon>().pixmap(128, 128));
          drag->exec();
        }
      }
    }
  }
}
