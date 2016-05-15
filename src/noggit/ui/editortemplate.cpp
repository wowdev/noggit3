// MainWindow.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>
// Glararan <glararan@glararan.eu>

#include <noggit/ui/editortemplate.h>

#include <QMenu>
#include <QMenuBar>

namespace noggit
{
  namespace ui
  {
    EditorTemplate::EditorTemplate(MainWindow* parent)
    : QWidget (nullptr)
    {
      mainWindow = parent;
      layout = new QGridLayout (this);
      editor = new QWidget (/*dummy*/);
      layout->addWidget (editor, 1, 1, Qt::AlignHorizontal_Mask);

//    QWidget *test = new QWidget();
//    test->setMaximumHeight(50);
//    test->show();

//    layout->addWidget(test,2,0,2,1);

      layout->setHorizontalSpacing (2);

      layout->setRowStretch (0, 1);
      layout->setRowStretch (1, 100);
      layout->setRowStretch (2, 1);

      layout->setColumnStretch (0, 1);
      layout->setColumnStretch (1, 100);
      layout->setColumnStretch (2, 1);
    }

    void EditorTemplate::addPropBar(QToolBar* bar)
    {
      layout->addWidget (bar, 0, 0, 1, 3, Qt::AlignTop);
    }

    void EditorTemplate::addToolBar(QToolBar* bar)
    {
      layout->addWidget (bar, 1, 0, 2, 1, Qt::AlignLeft | Qt::AlignTop);
    }

    void EditorTemplate::setEditor(QWidget* newEditor)
    {
      editor->hide();
      layout->removeWidget (editor);

      delete editor;

      editor = newEditor;
      editor->setParent (this);
      editor->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
      emit parentChanged();

      layout->addWidget (editor, 1, 1);
      editor->show();
    }

    void EditorTemplate::addEditorMenu(QMenu* menu)
    {
      mainWindow->menuBar()->addMenu (menu);
    }
  }
}
