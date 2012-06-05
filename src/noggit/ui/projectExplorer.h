// MainWindow.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>
// Glararan <glararan@glararan.eu>

#ifndef PROJECTEXPLORER_H
#define PROJECTEXPLORER_H

#include <QWidget>

namespace noggit
{
  namespace ui
  {
    class projectExplorer : public QWidget
    {
      Q_OBJECT

    public:
      explicit projectExplorer(const QString& projectPath, QWidget* parent = 0);
    };
  }
}

#endif // PROJECTEXPLORER_H
