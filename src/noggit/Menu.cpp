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
#include <noggit/ui/MenuBar.h> // UIMenuBar, menu items, ..
#include <noggit/ui/minimap_widget.hpp>
#include <noggit/ui/StatusBar.h> // UIStatusBar
#include <noggit/ui/uid_fix_window.hpp>
#include <noggit/uid_storage.hpp>
#ifdef USE_MYSQL_UID_STORAGE
  #include <mysql/mysql.h>
#endif

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

Menu::Menu()
	: mGUImenuBar(nullptr)
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

  if (mGUImenuBar)
  {
    mGUImenuBar->render();
  }
}

UIFrame::Ptr LastClickedMenu = nullptr;

void Menu::mouseReleaseEvent (SDL_MouseButtonEvent* e)
{
  if (e->button != SDL_BUTTON_LEFT)
  {
    return;
  }

  if (LastClickedMenu)
  {
    LastClickedMenu->processUnclick();
  }

  LastClickedMenu = nullptr;
}

void Menu::mousePressEvent (SDL_MouseButtonEvent* e)
{
  if (e->button != SDL_BUTTON_LEFT)
  {
    return;
  }

  LastClickedMenu = mGUImenuBar->processLeftClick(e->x, e->y);
}

void Menu::resizewindow()
{
  mGUImenuBar->resize();
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
  mGUImenuBar.reset (new UIMenuBar());
  mGUImenuBar->AddMenu("File");
  mGUImenuBar->GetMenu("File")->AddMenuItemButton
    ( "Settings"
    , [this]
      {
        auto mGUISettingsWindow (new UISettings());
        mGUISettingsWindow->readInValues();
        mGUISettingsWindow->show();
      }
    );
  mGUImenuBar->GetMenu("File")->AddMenuItemButton
    ( "About"
    , [this]
      {
        auto mGUICreditsWindow (new UIAbout());
        mGUICreditsWindow->show();
      }
    );
  mGUImenuBar->GetMenu("File")->AddMenuItemSwitch("exit ESC", &app.pop, true);

  static const char* typeToName[] = { "Continent", "Dungeons", "Raid", "Battleground", "Arena" };

  for (int i = 0; i < (sizeof(typeToName) / sizeof(*typeToName)); ++i)
  {
    mGUImenuBar->AddMenu(typeToName[i]);
  }

  for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
  {
    MapEntry e;
    e.mapID = i->getInt(MapDB::MapID);
    e.name = i->getLocalizedString(MapDB::Name);
    e.areaType = i->getUInt(MapDB::AreaType);
    if (e.areaType == 3) e.name = i->getString(MapDB::InternalName);

    if (e.areaType < 0 || e.areaType > 4 || !World::IsEditableWorld(e.mapID))
      continue;

    auto const map_id (e.mapID);

    mGUImenuBar->GetMenu(typeToName[e.areaType])->AddMenuItemButton(e.name, [e, this] { loadMap (e.mapID); });
  }

  if (mBookmarks.size())
  {
    mGUImenuBar->AddMenu("Bookmarks");
  }

  for (auto entry : mBookmarks)
  {
    mGUImenuBar->GetMenu("Bookmarks")->AddMenuItemButton
      ( entry.name
      , [entry, this]
        {
          loadMap (entry.mapID);
          enterMapAt (entry.pos, entry.av, entry.ah);
        }
      );
  }
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
