// MainWindow.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>
// Glararan <glararan@glararan.eu>

#ifndef EDITORTEMPLATE_H
#define EDITORTEMPLATE_H

#include <noggit/ui/MainWindow.h>

#include <QWidget>
#include <QToolBar>
#include <QGridLayout>

namespace noggit
{
  namespace ui
  {
    class EditorTemplate : public QWidget
    {
      Q_OBJECT

    public:
      explicit EditorTemplate(MainWindow* parent);

      void addPropBar(QToolBar* bar);
      void addToolBar(QToolBar* bar);
      void addEditorMenu(QMenu* menu);
      void setEditor(QWidget* newEditor);

    private:
      QGridLayout* layout;
      QWidget* editor;
      MainWindow* mainWindow;

    signals:
      void parentChanged();
    };
  }
}

#endif // EDITORTEMPLATE_H
