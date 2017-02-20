// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/MapView.h>

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QWidget>
#include <QDockWidget>

class UIModelImport : public QDockWidget
{
private:
  QListWidget* _list;
  QLineEdit* _textBox;

public:
  UIModelImport (MapView *mapview);
  void buildModelList();
};
