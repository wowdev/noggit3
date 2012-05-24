#ifndef EDITORTEMPLATE_H
#define EDITORTEMPLATE_H

#include <QWidget>
#include <QToolBar>
#include <QGridLayout>

#include "noggit/MainWindow.h"

namespace noggit
{

class EditorTemplate : public QWidget
{
    Q_OBJECT
public:
    explicit EditorTemplate(MainWindow *parent);
    void addPropBar(QToolBar *bar);
    void addToolBar(QToolBar *bar);
    void addEditorMenu(QMenu *menu);
    void setEditor(QWidget *newEditor);

private:
    QGridLayout *layout;
    QWidget *editor;
    MainWindow *mainWindow;
    
signals:
    void parentChanged();
    
public slots:
    
};

}

#endif // EDITORTEMPLATE_H
