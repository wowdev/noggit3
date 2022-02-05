#include <noggit/ui/main_window.hpp>

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/World.h>
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
#ifdef NOGGIT_HAS_SCRIPTING
#include <noggit/scripting/scripting_tool.hpp>
#endif

#ifdef USE_MYSQL_UID_STORAGE
  #include <mysql/mysql.h>

  #include <QtCore/QSettings>
#endif

#include "revision.h"


namespace noggit
{
  namespace ui
  {
    main_window::main_window()
      : QMainWindow (nullptr)
      , _null_widget (new QWidget (this))
    {
      std::stringstream title;
      title << "Noggit - " << STRPRODUCTVER;
#ifdef TESTING_VERSION
      title << " -- UNOFFICIAL TESTING VERSION";
#endif
      setWindowTitle (QString::fromStdString (title.str()));
      setWindowIcon (QIcon (":/icon"));

      setCentralWidget (_null_widget);

      createBookmarkList();

      _settings = new settings(this);
      _about = new about(this);

      auto file_menu (menuBar()->addMenu ("&Noggit"));

      auto settings_action (file_menu->addAction ("Settings"));
      QObject::connect ( settings_action, &QAction::triggered
                       , [&]
                         {
                           _settings->show();
                         }
                       );

      auto about_action (file_menu->addAction ("About"));
      QObject::connect ( about_action, &QAction::triggered
                       , [&]
                         {
                           _about->show();
                         }
                       );

      auto mapmenu_action (file_menu->addAction ("Exit"));
      QObject::connect ( mapmenu_action, &QAction::triggered
                       , [this]
                         {
                            close();
                         }
                       );

      build_menu();
    }

    void main_window::check_uid_then_enter_map
                        ( math::vector_3d pos
                        , math::degrees camera_pitch
                        , math::degrees camera_yaw
                        , bool from_bookmark
                        )
    {
      QSettings settings;
#ifdef USE_MYSQL_UID_STORAGE
      bool use_mysql = settings.value("project/mysql/enabled", false).toBool();

      if ((use_myqsl && mysql::hasMaxUIDStoredDB(_world->getMapID()))
        || uid_storage::hasMaxUIDStored(_world->getMapID())
         )
      {
        _world->mapIndex.loadMaxUID();
        enterMapAt(pos, camera_pitch, camera_yaw, from_bookmark);
      }
#else
      if (uid_storage::hasMaxUIDStored(_world->getMapID()))
      {
        if (settings.value("uid_startup_check", true).toBool())
        {
          enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::max_uid, from_bookmark);
        }
        else
        {
          _world->mapIndex.loadMaxUID();
          enterMapAt(pos, camera_pitch, camera_yaw, uid_fix_mode::none, from_bookmark);
        }
      }
#endif
      else
      {
        auto uidFixWindow(new uid_fix_window());
        uidFixWindow->show();

        connect( uidFixWindow
               , &noggit::ui::uid_fix_window::fix_uid
               , [this, pos, camera_pitch, camera_yaw, from_bookmark] (uid_fix_mode uid_fix)
                 {
                   enterMapAt(pos, camera_pitch, camera_yaw, uid_fix, from_bookmark);
                 }
               );
      }
    }

    void main_window::enterMapAt ( math::vector_3d pos
                                 , math::degrees camera_pitch
                                 , math::degrees camera_yaw
                                 , uid_fix_mode uid_fix
                                 , bool from_bookmark
                                 )
    {
      auto mapview (new MapView (camera_yaw, camera_pitch, pos, this, std::move (_world), uid_fix, from_bookmark));
      connect(mapview, &MapView::uid_fix_failed, [this]() { prompt_uid_fix_failure(); });

      map_loaded = true;

      setCentralWidget (mapview);
    }

    void main_window::loadMap(int mapID)
    {
      _minimap->world (nullptr);

      _world.reset();

      for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
      {
        if (it->getInt(MapDB::MapID) == mapID)
        {
          _world = std::make_unique<World> (it->getString(MapDB::InternalName), mapID);
          _minimap->world (_world.get());

          return;
        }
      }

      LogError << "Map with ID " << mapID << " not found. Failed loading." << std::endl;
    }

    void main_window::build_menu()
    {
      auto widget (new QWidget(this));
      auto layout (new QHBoxLayout (widget));

      QListWidget* continents_table (new QListWidget (widget));
      QListWidget* dungeons_table (new QListWidget (widget));
      QListWidget* raids_table (new QListWidget (widget));
      QListWidget* battlegrounds_table (new QListWidget (widget));
      QListWidget* arenas_table (new QListWidget (widget));
      QListWidget* bookmarks_table (new QListWidget (widget));

      QTabWidget* entry_points_tabs (new QTabWidget (widget));
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

        if (e.areaType < 0 || e.areaType > 4 || !World::IsEditableWorld(e.mapID))
          continue;

        auto item (new QListWidgetItem (QString::number(e.mapID) + " - " + QString::fromUtf8 (e.name.c_str()), type_to_table[e.areaType]));
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

                           _world.reset();

                           for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
                           {
                             if (it->getInt(MapDB::MapID) == entry.mapID)
                             {
                               _world = std::make_unique<World> (it->getString(MapDB::InternalName), entry.mapID);
                               check_uid_then_enter_map ( entry.pos
                                                        , math::degrees (entry.camera_pitch)
                                                        , math::degrees (entry.camera_yaw)
                                                        , true
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
        , [this] (::math::vector_3d const& pos)
          {
            check_uid_then_enter_map(pos, math::degrees(30.f), math::degrees(90.f));
          }
        );

      layout->addWidget (_minimap);

      setCentralWidget (widget);
    }

    void main_window::rebuild_menu()
    {
      setCentralWidget(_null_widget = new QWidget(this));

      createBookmarkList();
      build_menu();
      map_loaded = false;
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
      std::size_t areaID;
      BookmarkEntry b;
      int mapID = -1;
      while (f >> mapID >> b.pos.x >> b.pos.y >> b.pos.z >> b.camera_yaw >> b.camera_pitch >> areaID)
      {
        if (mapID == -1)
        {
          continue;
        }

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
      if (map_loaded)
      {
        event->ignore();
        prompt_exit(event);
      }
      else
      {
        event->accept();
      }
    }

    void main_window::prompt_exit(QCloseEvent* event)
    {
      emit exit_prompt_opened();

      QMessageBox prompt;
      prompt.setIcon (QMessageBox::Warning);
      prompt.setWindowFlags(Qt::WindowStaysOnTopHint);
      prompt.setText ("Exit?");
      prompt.setInformativeText ("Any unsaved changes will be lost.");
      prompt.addButton ("Exit", QMessageBox::DestructiveRole);
      prompt.addButton ("Return to menu", QMessageBox::AcceptRole);
      prompt.setDefaultButton (prompt.addButton ("Cancel", QMessageBox::RejectRole));
      prompt.setWindowFlags (Qt::CustomizeWindowHint | Qt::WindowTitleHint);


      prompt.exec();

      switch (prompt.buttonRole(prompt.clickedButton()))
      {
        case QMessageBox::AcceptRole:
          rebuild_menu();
          break;
        case QMessageBox::DestructiveRole:
          setCentralWidget(_null_widget = new QWidget(this));
          event->accept();
          break;
        default:
          event->ignore();
          break;
      }
    }

    void main_window::prompt_uid_fix_failure()
    {
      rebuild_menu();

      QMessageBox::critical
        ( nullptr
        , "UID fix failed"
        , "The UID fix couldn't be done because some models were missing or fucked up.\n"
          "The models are listed in the log file."
        , QMessageBox::Ok
        );
    }
  }
}
