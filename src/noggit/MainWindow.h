#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>
#include <QDockWidget>
#include <QMdiArea>

#include <noggit/Menu.h>
#include <noggit/MapView.h>

namespace noggit
{

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    
signals:
    
public slots:
    void create_world_view (World*);

private:
    void initialize_video();
    void createDockWidgets();

    QGLWidget* _dummy_gl_widget;
    QMdiArea *mdiArea;
    QToolBar *currentToolBar;
    
};

}

#endif // MAINWINDOW_H
