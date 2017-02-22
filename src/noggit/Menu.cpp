// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <noggit/MapView.h>
#include <noggit/Menu.h>
#include <noggit/Misc.h>
#include <noggit/Settings.h>
#include <noggit/Settings.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMOInstance.h> // WMOInstance (only for loading WMO only maps, we never load..)
#include <noggit/WMOInstance.h> // WMOInstance (only for loading WMO only maps, we never load..)
#include <noggit/World.h>
#include <noggit/World.h>
#include <noggit/application.h> // fonts, APP_*
#include <noggit/map_index.hpp>
#include <noggit/ui/About.h> // UIAbout
#include <noggit/ui/SettingsPanel.h> //UISettings
#include <noggit/ui/Frame.h> // UIFrame
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/StatusBar.h> // UIStatusBar
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/uid_storage.hpp>
#ifdef USE_MYSQL_UID_STORAGE
  #include <mysql/mysql.h>
#endif

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

Menu::Menu()
{
  gWorld = nullptr;

	createBookmarkList();
	buildMenuBar();

  addHotkey ( SDLK_ESCAPE
            , MOD_none
            , [this]
              {
                if (gWorld)
                {
                  delete gWorld;
                  gWorld = nullptr;
                }
                else
                {
                  app.pop = true;
                }
              }
            );
}

Menu::~Menu()
{
  delete gWorld;
  gWorld = nullptr;
}

void Menu::enterMapAt(math::vector_3d pos, float av, float ah)
{
  video.farclip((const float)Settings::getInstance()->FarZ);

  gWorld->camera = math::vector_3d(pos.x, pos.y, pos.z);

  gWorld->initDisplay();
  gWorld->mapIndex.enterTile(tile_index(pos));

  app.getStates().push_back(new MapView(ah, av, math::vector_3d(pos.x, pos.y, pos.z - 1.0f))); // on gPop, MapView is deleted.
}

void Menu::display(float /*t*/, float /*dt*/)
{
  video.set2D();
  gl.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Menu::loadMap(int mapID)
{
  delete gWorld;
  gWorld = nullptr;

	for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
	{
		if (it->getInt(MapDB::MapID) == mapID)
		{
      gWorld = new World(it->getString(MapDB::InternalName));
      auto mmw (new noggit::ui::minimap_widget (nullptr));
      mmw->world (gWorld);
      mmw->draw_boundaries (true);
      mmw->show();
      QObject::connect
        ( mmw,  &noggit::ui::minimap_widget::map_clicked
        , [this, mmw] (World* world, ::math::vector_3d const& pos)
          {
#ifdef USE_MYSQL_UID_STORAGE
            if ( Settings::getInstance()->mysql
               && mysql::hasMaxUIDStoredDB (*Settings::getInstance()->mysql, world->mMapId)
               )
            {
              world->mapIndex.loadMaxUID();
              enterMapAt (pos);
            }
            else
#endif
            if (uid_storage::getInstance()->hasMaxUIDStored(world->mMapId))
            {
              world->mapIndex.loadMaxUID();
              enterMapAt (pos);
            }
            else
            {
              auto uidFixWindow (new ui::uid_fix_window ([this, pos] { enterMapAt (pos); }));
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

void Menu::buildMenuBar()
{
  auto widget (new QWidget);
  auto layout (new QVBoxLayout (widget));
  auto button_layout (new QHBoxLayout);
  layout->addLayout (button_layout);

  auto settings_button (new QPushButton ("Settings", widget));
  button_layout->addWidget (settings_button);
  QObject::connect ( settings_button, &QPushButton::clicked
                   , []
                     {
                      auto mGUISettingsWindow (new UISettings());
                      mGUISettingsWindow->readInValues();
                      mGUISettingsWindow->show();
                     }
                   );

  auto about_button (new QPushButton ("About", widget));
  button_layout->addWidget (about_button);
  QObject::connect ( about_button, &QPushButton::clicked
                   , []
                     {
                       auto mGUICreditsWindow (new UIAbout());
                       mGUICreditsWindow->show();
                     }
                   );

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
                     , [this] (QListWidgetItem* item) { loadMap (item->data (Qt::UserRole).toInt()); }
                     );
  }

  QObject::connect ( bookmarks_table, &QListWidget::itemDoubleClicked
                   , [this] (QListWidgetItem* item)
                     {
                       auto& entry (mBookmarks.at (item->data (Qt::UserRole).toInt()));
                       enterMapAt (entry.pos, entry.av, entry.ah);
                     }
                   );

  widget->show();
}

void Menu::createBookmarkList()
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
