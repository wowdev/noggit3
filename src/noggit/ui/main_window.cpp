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

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenuBar>
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
    {
      createBookmarkList();

      auto file_menu (menuBar()->addMenu ("&File"));

      auto settings_action (file_menu->addAction ("Settings"));
      QObject::connect ( settings_action, &QAction::triggered
                       , []
                         {
                           auto settings (new UISettings());
                           settings->readInValues();
                           settings->show();
                         }
                       );

      auto about_action (file_menu->addAction ("About"));
      QObject::connect ( about_action, &QAction::triggered
                       , []
                         {
                           auto about (new UIAbout());
                           about->show();
                         }
                       );

      auto mapmenu_action (file_menu->addAction ("Load Map"));
      QObject::connect ( mapmenu_action, &QAction::triggered
                       , [this]
                         {
                           build_menu();
                         }
                       );
    }

    void main_window::enterMapAt(math::vector_3d pos, float av, float ah)
    {
      app.start_main_loop();

      video.farclip (Settings::getInstance()->FarZ);

      gWorld->camera = math::vector_3d (pos.x, pos.y, pos.z);

      gWorld->initDisplay();
      gWorld->mapIndex.enterTile (tile_index (pos));

      app.getStates().push_back
        (new MapView (ah, av, math::vector_3d (pos.x, pos.y, pos.z - 1.0f)));
    }

    void main_window::loadMap(int mapID)
    {
      delete gWorld;
      gWorld = nullptr;

      for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
      {
        if (it->getInt(MapDB::MapID) == mapID)
        {
          gWorld = new World(it->getString(MapDB::InternalName));
          auto mmw (new minimap_widget (nullptr));
          mmw->world (gWorld);
          mmw->draw_boundaries (true);
          mmw->show();
          QObject::connect
            ( mmw,  &minimap_widget::map_clicked
            , [this, mmw] (World* world, ::math::vector_3d const& pos)
              {
#ifdef USE_MYSQL_UID_STORAGE
                if ( Settings::getInstance()->mysql
                   && mysql::hasMaxUIDStoredDB (*Settings::getInstance()->mysql, world->mMapId)
                   )
                {
                  world->mapIndex.loadMaxUID();
                  enterMapAt (pos, -30.f, -90.f);
                }
                else
#endif
                if (uid_storage::getInstance()->hasMaxUIDStored(world->mMapId))
                {
                  world->mapIndex.loadMaxUID();
                  enterMapAt (pos, -30.f, -90.f);
                }
                else
                {
                  auto uidFixWindow
                    (new ::ui::uid_fix_window ([this, pos] { enterMapAt (pos, -30.f, -90.f); }));
                  uidFixWindow->show();
                }
                mmw->deleteLater();
              }
            );

          return;
        }
      }

      LogError << "Map with ID " << mapID << " not found. Failed loading." << std::endl;
    }

    void main_window::build_menu()
    {
      auto widget (new QWidget);
      auto layout (new QVBoxLayout (widget));

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
        {continents_table, dungeons_table, raids_table, battlegrounds_table, arenas_table};

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
                         , [this, widget] (QListWidgetItem* item)
                           {
                             loadMap (item->data (Qt::UserRole).toInt());
                             widget->deleteLater();
                           }
                         );
      }

      QObject::connect ( bookmarks_table, &QListWidget::itemDoubleClicked
                       , [this, widget] (QListWidgetItem* item)
                         {
                           auto& entry (mBookmarks.at (item->data (Qt::UserRole).toInt()));
                           enterMapAt (entry.pos, entry.av, entry.ah);
                           widget->deleteLater();
                         }
                       );

      widget->show();
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
      while (f >> mapID >> b.pos.x >> b.pos.y >> b.pos.z >> b.ah >> b.av >> areaID)
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
  }
}
