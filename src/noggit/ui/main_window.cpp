#include <noggit/ui/main_window.hpp>

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/World.h>
#include <noggit/Settings.h>
#include <noggit/application.h>
#include <noggit/ui/About.h>
#include <noggit/MapView.h>
#include <noggit/ui/SettingsPanel.h>
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/uid_storage.hpp>

#include <QtGui/QCloseEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#ifdef USE_MYSQL_UID_STORAGE
  #include <mysql/mysql.h>
#endif

namespace noggit
{
  namespace ui
  {
    main_window::main_window()
      : QMainWindow (nullptr)
      , _null_widget (new QWidget (this))
    {
      setCentralWidget (_null_widget);

      createBookmarkList();

      auto file_menu (menuBar()->addMenu ("&Noggit"));

      auto settings_action (file_menu->addAction ("Settings"));
      QObject::connect ( settings_action, &QAction::triggered
                       , []
                         {
                           auto window (new settings());
                           window->readInValues();
                           window->show();
                         }
                       );

      auto about_action (file_menu->addAction ("About"));
      QObject::connect ( about_action, &QAction::triggered
                       , []
                         {
                           auto window (new about());
                           window->show();
                         }
                       );

      auto mapmenu_action (file_menu->addAction ("Create Map Editor"));
      QObject::connect ( mapmenu_action, &QAction::triggered
                       , [this]
                         {
                           build_menu();
                         }
                       );

      build_menu();
    }

    void main_window::enterMapAt ( math::vector_3d pos
                                 , math::degrees camera_pitch
                                 , math::degrees camera_yaw
                                 , World* world
                                 )
    {
      auto mapview (new MapView (camera_yaw, camera_pitch, pos, this, world));
      setCentralWidget (mapview);
    }

    void main_window::loadMap(int mapID)
    {
      _minimap->world (nullptr);
      delete gWorld;
      gWorld = nullptr;

      for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
      {
        if (it->getInt(MapDB::MapID) == mapID)
        {
          gWorld = new World(it->getString(MapDB::InternalName), mapID);
          _minimap->world (gWorld);

          return;
        }
      }

      LogError << "Map with ID " << mapID << " not found. Failed loading." << std::endl;
    }

    void main_window::build_menu()
    {
      auto widget (new QWidget);
      auto layout (new QHBoxLayout (widget));

      QListWidget* continents_table (new QListWidget (nullptr));
      QListWidget* dungeons_table (new QListWidget (nullptr));
      QListWidget* raids_table (new QListWidget (nullptr));
      QListWidget* battlegrounds_table (new QListWidget (nullptr));
      QListWidget* arenas_table (new QListWidget (nullptr));
      QListWidget* bookmarks_table (new QListWidget (nullptr));

      QTabWidget* entry_points_tabs (new QTabWidget (nullptr));
      entry_points_tabs->addTab (continents_table, "Continents");
      entry_points_tabs->addTab (dungeons_table, "Dungeons");
      entry_points_tabs->addTab (raids_table, "Raids");
      entry_points_tabs->addTab (battlegrounds_table, "Battlegrounds");
      entry_points_tabs->addTab (arenas_table, "Arenas");
      entry_points_tabs->addTab (bookmarks_table, "Bookmarks");

      layout->addWidget (entry_points_tabs);

      std::array<QListWidget*, 5> type_to_table
        {{continents_table, dungeons_table, raids_table, battlegrounds_table, arenas_table}};

      for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
      {
        MapEntry e;
        e.mapID = i->getInt(MapDB::MapID);
        e.name = i->getLocalizedString(MapDB::Name);
        e.areaType = i->getUInt(MapDB::AreaType);
        if (e.areaType == 3) e.name = i->getString(MapDB::InternalName);

        if (e.areaType < 0 || e.areaType > 4 || !World::IsEditableWorld(e.mapID))
          continue;

        auto item (new QListWidgetItem (QString::fromUtf8 (e.name.c_str()), type_to_table[e.areaType]));
        item->setData (Qt::UserRole, QVariant (e.mapID));
      }

      qulonglong bookmark_index (0);
      for (auto entry : mBookmarks)
      {
        auto item (new QListWidgetItem (entry.name.c_str(), bookmarks_table));
        item->setData (Qt::UserRole, QVariant (bookmark_index++));
      }

      for (auto& table : type_to_table)
      {
        QObject::connect ( table, &QListWidget::itemClicked
                         , [this] (QListWidgetItem* item)
                           {
                             loadMap (item->data (Qt::UserRole).toInt());
                           }
                         );
      }

      QObject::connect ( bookmarks_table, &QListWidget::itemDoubleClicked
                       , [this] (QListWidgetItem* item)
                         {
                           auto& entry (mBookmarks.at (item->data (Qt::UserRole).toInt()));

                           for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
                           {
                             if (it->getInt(MapDB::MapID) == entry.mapID)
                             {
                               gWorld = new World(it->getString(MapDB::InternalName), entry.mapID);
                               enterMapAt ( entry.pos
                                          , math::degrees (entry.camera_pitch)
                                          , math::degrees (entry.camera_yaw)
                                          , gWorld
                                          );

                               return;
                             }
                           }
                         }
                       );


      _minimap = new minimap_widget (this);
      _minimap->draw_boundaries (true);

      QObject::connect
        ( _minimap,  &minimap_widget::map_clicked
        , [this] (World* world, ::math::vector_3d const& pos)
          {
#ifdef USE_MYSQL_UID_STORAGE
            if ( Settings::getInstance()->mysql
               && mysql::hasMaxUIDStoredDB (*Settings::getInstance()->mysql, world->getMapID())
               )
            {
              world->mapIndex.loadMaxUID();
              enterMapAt (pos, math::degrees (30.f), math::degrees (90.f), world);
            }
            else
#endif
            if (uid_storage::getInstance()->hasMaxUIDStored(world->getMapID()))
            {
              world->mapIndex.loadMaxUID();
              enterMapAt (pos, math::degrees (30.f), math::degrees (90.f), world);
            }
            else
            {
              auto uidFixWindow
                ( new noggit::ui::uid_fix_window
                    ( [this, pos, world]
                      {
                        enterMapAt (pos, math::degrees (30.f), math::degrees (90.f), world);
                      }
                    , world
                    )
                );
              uidFixWindow->show();
            }
          }
        );

      layout->addWidget (_minimap);

      setCentralWidget (widget);
    }

    void main_window::createBookmarkList()
    {
      mBookmarks.clear();

      std::ifstream f("bookmarks.txt");
      if (!f.is_open())
      {
        LogDebug << "No bookmarks file." << std::endl;
        return;
      }

      std::string basename;
      int areaID;
      BookmarkEntry b;
      int mapID = -1;
      while (f >> mapID >> b.pos.x >> b.pos.y >> b.pos.z >> b.camera_yaw >> b.camera_pitch >> areaID)
      {
        if (mapID == -1)
          continue;

        std::stringstream temp;
        temp << MapDB::getMapName(mapID) << ": " << AreaDB::getAreaName(areaID);
        b.name = temp.str();
        b.mapID = mapID;
        mBookmarks.push_back(b);
      }
      f.close();
    }

    void main_window::closeEvent (QCloseEvent* event)
    {
      if (centralWidget() != _null_widget)
      {
        event->ignore();
        prompt_exit();
      }
    }

    void main_window::prompt_exit()
    {
      QMessageBox prompt;
      prompt.setIcon (QMessageBox::Warning);
      prompt.setWindowTitle ("Exit current editor?");
      prompt.setText ("Exit current editor?");
      prompt.setInformativeText ("Any unsaved changes will be lost.");
      prompt.addButton ("Exit", QMessageBox::AcceptRole);
      prompt.setDefaultButton (prompt.addButton ("Continue Editing", QMessageBox::RejectRole));
      prompt.setWindowFlags (Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      prompt.exec();

      if (prompt.buttonRole (prompt.clickedButton()) == QMessageBox::AcceptRole)
      {
        setCentralWidget (_null_widget = new QWidget (this));
      }
    }
  }
}
