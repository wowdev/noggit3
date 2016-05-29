// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QMainWindow>
#include <QVariant>
#include <QDockWidget>

#include <noggit/Menu.h>
#include <noggit/MapView.h>

namespace noggit
{
  namespace ui
  {
    class settingsDialog;
    class about_widget;
    class help_widget;

    class MainWindow : public QMainWindow
    {
      Q_OBJECT

    public:
      MainWindow(QWidget* parent = 0);

      void addEditorMenu(QMenu* menu);

	public:
		Menu* map_selection_menu;

    public slots:
      void create_world_view (World*);

    private slots:
      void settingsClicked();

      void maps();
      void projectExplorerOpen();

      void help_mapEditor();
      void about();

    private:
      void initialize_video();
      void createDockWidgets();

#ifdef Q_OS_MAC
      QMenuBar* _menu_bar;
      QMenuBar* menuBar();
#endif

      QToolBar* currentToolBar;

      noggit::ui::settingsDialog* _settings;
      noggit::ui::about_widget* _about;
      noggit::ui::help_widget* _help;
      World* _world;
    };
  }
}
