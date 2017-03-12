// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>

namespace noggit
{
  namespace ui
  {
    class widget : public QWidget
    {
      Q_OBJECT

    public:
      using QWidget::QWidget;

      virtual void showEvent (QShowEvent* event) override
      {
        emit visibilityChanged (true);
        return QWidget::showEvent (event);
      }
      virtual void hideEvent (QHideEvent* event) override
      {
        emit visibilityChanged (false);
        return QWidget::hideEvent (event);
      }

    signals:
      void visibilityChanged (bool);
    };
  }
}
