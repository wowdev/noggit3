// MainWindow.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>
// Glararan <glararan@glararan.eu>

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
  namespace ui
  {
    class settingsDialog;
  }

  class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0);
    
  signals:

  public slots:
    void create_world_view (World*);

  private slots:
    void settingsClicked();

    void maps();
    void projectExplorerOpen();

  private:
    void initialize_video();
    void createDockWidgets();

    QGLWidget* _dummy_gl_widget;
    QMdiArea* mdiArea;
    QToolBar* currentToolBar;

    noggit::ui::settingsDialog* _settings;
    World* _world;
  };
}

#endif // MAINWINDOW_H
