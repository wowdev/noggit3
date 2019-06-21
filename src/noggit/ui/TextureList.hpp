// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/widget.hpp>
#include <QtWidgets/QListView>

class QMouseEvent;

namespace noggit
{
  namespace ui
  {
    class TextureList : public QListView
    {
      Q_OBJECT

    public:
      TextureList(QWidget* parent = nullptr);

      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      QPoint _start_pos;

    };
  }
}
