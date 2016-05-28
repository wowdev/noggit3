// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MainWindow.h"

#include <stdexcept>

#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>

#include <noggit/Log.h>
#include <noggit/application.h>
#include <noggit/ui/editortemplate.h>
#include <noggit/ui/projectExplorer.h>
#include <noggit/ui/textureselecter.h>
#include <noggit/ui/settingsDialog.h>
#include <noggit/ui/about_widget.h>
#include <noggit/ui/help_widget.h>
#include <noggit/World.h>

namespace noggit
{
  namespace ui
  {
    MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
#ifdef Q_WS_MAC
    //! \note do not set parent to have global menu bar for all windows so long editor is not part of mainwindow
    , _menu_bar (new QMenuBar (nullptr))
#else
    , _menu_bar (new QMenuBar (this))
#endif
    {
      setWindowTitle("NoggIt Studio");

      //textureSelecter *test = new textureSelecter();

      QMenu* fileMenu = menuBar()->addMenu (tr("&File"));
      fileMenu->addAction (tr("Open Maps"), this, SLOT(maps()));
      fileMenu->addAction (tr("Open Project Explorer"), this, SLOT(projectExplorerOpen()));
      fileMenu->addSeparator();
      fileMenu->addAction (tr("Exit"), &noggit::app(), SLOT(closeAllWindows()));

    QMenu* helpMenu = menuBar()->addMenu (tr("&Help"));
      helpMenu->addAction (tr("Settings"), this, SLOT(settingsClicked()));
      helpMenu->addSeparator();
      helpMenu->addAction (tr("Map Editor"), this, SLOT(help_mapEditor()));
      helpMenu->addAction (tr("About"), this, SLOT(about()));

      //QMenu* debugMenu = menuBar()->addMenu (tr("&Debug"));
      //debugMenu->addAction (tr("textureSelector"), test, SLOT(show()));

      if(noggit::app().setting("projectExplorerShow").toBool() == true)
      createDockWidgets();

      statusBar()->showMessage (tr("Ready"));

      maps();
    }

    QMenuBar* MainWindow::menuBar()
    {
      return _menu_bar;
    }

    void MainWindow::createDockWidgets()
    {
      projectExplorer* explorer = new projectExplorer(app().setting("paths/project").toString());
      QDockWidget* dockWidget = new QDockWidget(tr("Project Explorer"), this);
      dockWidget->setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      dockWidget->setWidget (explorer);
      this->addDockWidget (Qt::LeftDockWidgetArea, dockWidget);
    }

    void MainWindow::create_world_view (World* world)
    {

      MapView* map_view ( new MapView ( world
                                      , app().setting("view_distance").toReal()
                                      , 0.0
                                      , 0.0
                                      , this
                                      )
                          );

      noggit::ui::EditorTemplate* temp = new noggit::ui::EditorTemplate (this);
      connect(temp, SIGNAL(parentChanged()), map_view, SLOT(updateParent()));
      temp->setEditor (map_view);
      const int xResolution (app().setting("resolution/x").toInt());
      const int yResolution (app().setting("resolution/y").toInt());
      temp->resize (xResolution, yResolution);

      if(noggit::app().setting("maximizedShow").toBool() == false)
        temp->show();
      else
        temp->showMaximized();
    }

    void MainWindow::maps()
    {
      this->map_selection_menu = new Menu (nullptr);
      connect (this->map_selection_menu, SIGNAL (create_world_view_request (World*)), this, SLOT (create_world_view (World*)));
      connect (map_selection_menu, SIGNAL (create_world_view_request (World*)), map_selection_menu, SLOT (deleteLater()));
	  this->map_selection_menu->show();

    }

    void MainWindow::projectExplorerOpen()
    {
      createDockWidgets();
    }

    void MainWindow::settingsClicked()
    {
      _settings = new noggit::ui::settingsDialog;
      _settings->show();
    }

    void MainWindow::about()
    {
      _about = new noggit::ui::about_widget;
      _about->show();
    }

    void MainWindow::help_mapEditor()
    {
      _help = new noggit::ui::help_widget;
      _help->show();
    }
  }
}
