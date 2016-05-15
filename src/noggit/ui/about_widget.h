// about_widget.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef __WIN_CREDITS_H
#define __WIN_CREDITS_H

#include <QWidget>

namespace noggit
{
  namespace ui
  {
    class about_widget : public QWidget
    {
      Q_OBJECT

    public:
      about_widget (QWidget* parent = nullptr);
    };
  }
}

#endif
