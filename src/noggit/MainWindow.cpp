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
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMetaType>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>

#include <math/vector_3d.h>

#include <noggit/Log.h>
#include <noggit/application.h>
#include <noggit/DBC.h>
#include <noggit/MapView.h>
#include <noggit/World.h>
#include <noggit/editortemplate.h>
#include <noggit/ui/projectExplorer.h>
#include <noggit/ui/textureselecter.h>
#include <noggit/ui/settingsDialog.h>
#include <noggit/ui/minimap_widget.h>

struct bookmark_entry
{
  int map_id;
  ::math::vector_3d position;
  float rotation;
  float tilt;
};

// for storing in QVariant
Q_DECLARE_METATYPE (bookmark_entry);

namespace noggit
{
  MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , _minimap (new noggit::ui::minimap_widget (NULL))
  , _world (NULL)
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
	fileMenu->addSeparator();
    fileMenu->addAction (tr("Exit"), this, SLOT(close()));

	QMenu* helpMenu = menuBar()->addMenu (tr("&Help"));
	helpMenu->addAction (tr("Settings"), this, SLOT(settingsClicked()));

    //QMenu* debugMenu = menuBar()->addMenu (tr("&Debug"));
    //debugMenu->addAction (tr("textureSelector"), test, SLOT(show()));

    setCentralWidget (mdiArea);

    createDockWidgets();

    statusBar()->showMessage (tr("Ready"));
    currentToolBar = addToolBar (tr("File"));
    currentToolBar->addWidget (new QLabel(tr("Toolbar")));
  }

  MainWindow::~MainWindow()
  {
    delete _world;
    _world = NULL;
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
    temp->show();
  }

  void MainWindow::maps()
  {
    //prepare_maps();
    //connect (this, SIGNAL(create_world_view_request (World*)), this, SLOT (create_world_view (World*)));
    Menu* map_selection_menu (new Menu (NULL));
    connect (map_selection_menu, SIGNAL (create_world_view_request (World*)), this, SLOT (create_world_view (World*)));
    map_selection_menu->show();

    mdiArea->addSubWindow(map_selection_menu);
    map_selection_menu->show();
  }

//  void MainWindow::prepare_maps()
//  {
//    maps_widget = new QWidget();

//    QListWidget* continents_table (new QListWidget (NULL));
//    QListWidget* dungeons_table (new QListWidget (NULL));
//    QListWidget* raids_table (new QListWidget (NULL));

//    connect (continents_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_map_list_item (QListWidgetItem*)));
//    connect (dungeons_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_map_list_item (QListWidgetItem*)));
//    connect (raids_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_map_list_item (QListWidgetItem*)));

//    QListWidget* tables[3] = { continents_table, dungeons_table, raids_table };

//    for(DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
//    {
//      const int mapID (i->getInt (MapDB::MapID));
//      const int areaType (i->getUInt (MapDB::AreaType));
//      if(areaType < 0 || areaType > 2 || !World::IsEditableWorld (mapID))
//        continue;

//      QListWidgetItem* item (new QListWidgetItem (QString::fromUtf8 (i->getLocalizedString(MapDB::Name)), tables[areaType]));
//      item->setData (Qt::UserRole, QVariant (mapID));
//    }

//    QListWidget* bookmarks_table (new QListWidget (NULL));

//    connect (bookmarks_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_bookmark_list_item (QListWidgetItem*)));
//    connect (bookmarks_table, SIGNAL (itemDoubleClicked (QListWidgetItem*)), SLOT (open_bookmark_list_item (QListWidgetItem*)));

//    //! \todo The list needs to be refreshed upon adding a bookmark.
//    QSettings settings;

//    int bookmarks (settings.beginReadArray ("bookmarks"));
//    for(int i (0); i < bookmarks; ++i)
//    {
//      settings.setArrayIndex (i);
//      bookmark_entry b;
//      b.map_id = settings.value ("map_id").toInt();
//      b.position.x (settings.value ("camera/position/x").toFloat());
//      b.position.y (settings.value ("camera/position/y").toFloat());
//      b.position.z (settings.value ("camera/position/z").toFloat());
//      b.rotation = settings.value ("camera/rotation").toFloat();
//      b.tilt = settings.value ("camera/tilt").toFloat();

//      const int area_id (settings.value ("area_id").toInt());

//      QListWidgetItem* item (new QListWidgetItem (QString ("%1: %2").arg (MapDB::getMapName (b.map_id)).arg (AreaDB::getAreaName (area_id)), bookmarks_table));
//      item->setData (Qt::UserRole, QVariant::fromValue (b));
//    }
//    settings.endArray();

//    QGridLayout* menu_layout (new QGridLayout (this));

//    QTabWidget* entry_points_tabs (new QTabWidget (NULL));
//    entry_points_tabs->addTab (continents_table, tr ("Continents"));
//    entry_points_tabs->addTab (dungeons_table, tr ("Dungeons"));
//    entry_points_tabs->addTab (raids_table, tr ("Raids"));
//    entry_points_tabs->addTab (bookmarks_table, tr ("Bookmarks"));

//    _minimap->draw_boundaries (true);
//    connect (_minimap, SIGNAL (map_clicked (const ::math::vector_3d&)), SLOT (minimap_clicked (const ::math::vector_3d&)));

//    menu_layout->addWidget (entry_points_tabs, 0, 0);
//    menu_layout->addWidget (_minimap, 0, 1);

//	maps_widget->setLayout (menu_layout);

//    EditorTemplate* menu_temp = new EditorTemplate(this);
//    connect(menu_temp, SIGNAL(parentChanged()), maps_widget, SLOT(updateParent()));
//    menu_temp->setEditor (maps_widget);
//    mdiArea->addSubWindow (menu_temp);
//    menu_temp->show();
//  }

//  void MainWindow::enter_world_at (const ::math::vector_3d& pos, bool auto_height, float av, float ah)
//  {
//    prepare_world (pos, ah, av);
//    _world->enterTile (pos.x() / TILESIZE, pos.y() / TILESIZE);

//    emit create_world_view_request (_world);

//    if(auto_height)
//      _world->set_camera_above_terrain();

//	maps_widget->setVisible(false);
//  }

//  void MainWindow::load_map (int map_id)
//  {
//    if(_world && _world->getMapID() == map_id)
//      return;

//    delete _world;

//    _world = new World (gMapDB.getByID (map_id, MapDB::MapID).getString (MapDB::InternalName));

//    _minimap->world (_world);
//  }

//  void MainWindow::minimap_clicked (const ::math::vector_3d& position)
//  {
//    enter_world_at (position, true, 0.0, 0.0);
//  }

//  void MainWindow::prepare_world (const ::math::vector_3d& pos, float rotation, float tilt)
//  {
//    _world->camera = ::math::vector_3d (pos.x(), pos.y(), pos.z());
//    //! \todo actually set lookat!
//    _world->lookat = ::math::vector_3d (pos.x() + 10.0f, pos.y() + 10.0f, pos.z() + 10.0f); // ah = rotation

//    _world->initDisplay();
//  }

//  void MainWindow::show_map_list_item (QListWidgetItem* item)
//  {
//    load_map (item->data (Qt::UserRole).toInt());
//    _minimap->draw_camera (false);
//  }

//  void MainWindow::show_bookmark_list_item (QListWidgetItem* item)
//  {
//    const bookmark_entry e (item->data (Qt::UserRole).value<bookmark_entry>());
//    load_map (e.map_id);
//    prepare_world (e.position, e.rotation, e.tilt);
//    _minimap->draw_camera (true);
//  }

//  void MainWindow::open_bookmark_list_item (QListWidgetItem* item)
//  {
//    const bookmark_entry e (item->data (Qt::UserRole).value<bookmark_entry>());
//    load_map (e.map_id);
//    enter_world_at (e.position, false, e.tilt, e.rotation);
//  }

  void MainWindow::settingsClicked()
  {
    noggit::ui::settingsDialog* config (new noggit::ui::settingsDialog);
    config->show();
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
