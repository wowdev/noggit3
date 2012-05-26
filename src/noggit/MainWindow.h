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

class QKeyEvent;
class QMouseEvent;
class QListWidgetItem;
class QWidget;

class World;

namespace math
{
  class vector_3d;
}

namespace noggit
{
  namespace ui
  {
    class settingsDialog;
    class minimap_widget;
  }

  class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();
    
  signals:
    void create_world_view_request (World*);

  public slots:
    void create_world_view (World*);

  private slots:
    void show_map_list_item (QListWidgetItem* item);
    void show_bookmark_list_item (QListWidgetItem* item);
    void open_bookmark_list_item (QListWidgetItem* item);

    void minimap_clicked (const ::math::vector_3d&);
    void settingsClicked();

    void maps();

  private:
    void initialize_video();
    void createDockWidgets();

    QGLWidget* _dummy_gl_widget;
    QMdiArea* mdiArea;
    QToolBar* currentToolBar;

    noggit::ui::settingsDialog* _settings;
    noggit::ui::minimap_widget* _minimap;
    World* _world;
	QWidget* maps_widget;

    void prepare_maps();
    void load_map (int mapID);
    void prepare_world (const ::math::vector_3d& pos, float rotation, float tilt);
    void enter_world_at (const ::math::vector_3d& pos, bool auto_height = true, float av = -30.0f, float ah = -90.0f);
  };
}

#endif // MAINWINDOW_H
