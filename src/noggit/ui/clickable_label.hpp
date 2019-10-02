// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QLabel>

namespace noggit
{
  namespace ui
  {
    class clickable_label : public QLabel
    {
      Q_OBJECT

    public:
      clickable_label(QWidget* parent=nullptr);

    signals:
      void clicked();

    protected:
      virtual void mouseReleaseEvent (QMouseEvent* event) override;
    };
  }
}
