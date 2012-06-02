// MainWindow.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>
// Glararan <glararan@glararan.eu>

#include "MainWindow.h"

#include <stdexcept>

#include <QGLWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>

#include <noggit/Log.h>
#include <noggit/application.h>
#include <noggit/editortemplate.h>
#include <noggit/ui/projectExplorer.h>
#include <noggit/ui/textureselecter.h>
#include <noggit/ui/settingsDialog.h>
#include <noggit/ui/about_widget.h>
#include <noggit/ui/help_widget.h>

namespace noggit
{
  MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  {
    initialize_video();
    setWindowTitle("NoggIt Studio");

    const int xResolution (app().setting("resolution/x").toInt());
    const int yResolution (app().setting("resolution/y").toInt());
    this->resize (xResolution, yResolution);

    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy (Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy (Qt::ScrollBarAsNeeded);

    //textureSelecter *test = new textureSelecter(_dummy_gl_widget);
    //! todo windows has a problem with sharing the dummy (may sth about render contex)

    QMenu* fileMenu = menuBar()->addMenu (tr("&File"));
    fileMenu->addAction (tr("Open maps"), this, SLOT(maps()));
    fileMenu->addAction (tr("Open ProjectExplorer"), this, SLOT(projectExplorerOpen()));
    fileMenu->addSeparator();
    fileMenu->addAction (tr("Exit"), this, SLOT(close()));

	QMenu* helpMenu = menuBar()->addMenu (tr("&Help"));
	helpMenu->addAction (tr("Settings"), this, SLOT(settingsClicked()));
	helpMenu->addSeparator();
    helpMenu->addAction (tr("Map Editor"), this, SLOT(help_mapEditor()));
    helpMenu->addAction (tr("About"), this, SLOT(about()));

    //QMenu* debugMenu = menuBar()->addMenu (tr("&Debug"));
    //debugMenu->addAction (tr("textureSelector"), test, SLOT(show()));

    setCentralWidget (mdiArea);

    if(noggit::app().setting("projectExplorerShow").toBool() == true)
      createDockWidgets();

    statusBar()->showMessage (tr("Ready"));
    currentToolBar = addToolBar (tr("File"));
    currentToolBar->addWidget (new QLabel(tr("Toolbar")));
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
                                      , _dummy_gl_widget
                                      , this
                                      )
                        );

    EditorTemplate* temp = new EditorTemplate(this);
    connect(temp, SIGNAL(parentChanged()), map_view, SLOT(updateParent()));
    temp->setEditor (map_view);
    mdiArea->addSubWindow (temp);

    if(noggit::app().setting("maximizedShow").toBool() == false)
      temp->show();
    else
      temp->showMaximized();
  }

  void MainWindow::maps()
  {
    Menu* map_selection_menu (new Menu (NULL));
    connect (map_selection_menu, SIGNAL (create_world_view_request (World*)), this, SLOT (create_world_view (World*)));

    mdiArea->addSubWindow(map_selection_menu);

    if(noggit::app().setting("maximizedShow").toBool() == false)
      map_selection_menu->show();
    else
      map_selection_menu->showMaximized();
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


  class dummy_gl_widget : public QGLWidget
  {
  public:
    dummy_gl_widget (const QGLFormat& format)
    : QGLWidget (format)
    {
      updateGL();
    }

  protected:
    virtual void initializeGL()
    {
      const GLenum err (glewInit());
      if(GLEW_OK != err)
      {
        LogError << "GLEW: " << glewGetErrorString (err) << std::endl;
        throw std::runtime_error ("unable to initialize glew.");
      }

      //! \todo Fallback for old and bad platforms.
      if(!glGenBuffers)
		  glGenBuffers = glGenBuffersARB;
      if(!glBindBuffer)
		  glBindBuffer = glBindBufferARB;
      if(!glBufferData)
		  glBufferData = glBufferDataARB;

      LogDebug << "GL: Version: " << glGetString (GL_VERSION) << std::endl;
      LogDebug << "GL: Vendor: " << glGetString (GL_VENDOR) << std::endl;
      LogDebug << "GL: Renderer: " << glGetString (GL_RENDERER) << std::endl;
    }
  };

  void MainWindow::initialize_video()
  {
    if(!QGLFormat::hasOpenGL())
      LogError << "Your system does not support OpenGL. Sorry, this application can't run without it." << std::endl;

    QGLFormat format;
    format.setStencilBufferSize (1);
    format.setDepthBufferSize (16);
    format.setAlphaBufferSize (8);

    if(app().setting("antialiasing").toBool())
    {
      format.setSampleBuffers (true);
      format.setSamples (4);
    }

    _dummy_gl_widget = new dummy_gl_widget (format);
  }
}
