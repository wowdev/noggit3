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
#include <noggit/ui/MinimapWindow.h> // UIMinimapWindow
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
	: mGUIFrame(nullptr)
	, mGUICreditsWindow(nullptr)
    , mGUISettingsWindow(nullptr)
	, mGUIMinimapWindow(nullptr)
	, mGUImenuBar(nullptr)
  , uidFixWindow(nullptr)
{
  gWorld = nullptr;

  mGUIFrame = std::make_unique<UIFrame> (0.0f, 0.0f, (float)video.xres(), (float)video.yres());
  mGUIMinimapWindow = new UIMinimapWindow(this);
  mGUIMinimapWindow->hide();
  mGUIFrame->addChild(mGUIMinimapWindow);
  mGUICreditsWindow = new UIAbout();
  mGUISettingsWindow = new UISettings();

  uidFixWindow = new ui::uid_fix_window(this);

	createMapList();
	createBookmarkList();
	buildMenuBar();

  addHotkey ( SDLK_ESCAPE
            , MOD_none
            , [this]
              {
                if (gWorld)
                {
                  mGUIMinimapWindow->hide();
                  uidFixWindow->hide();
                  mGUICreditsWindow->show();
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

  mGUIMinimapWindow->hide();
}

void Menu::display(float /*t*/, float /*dt*/)
{
  video.set2D();
  gl.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  mGUIFrame->render();
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

  LastClickedMenu = mGUIFrame->processLeftClick(e->x, e->y);
}

void Menu::mousemove(SDL_MouseMotionEvent *e)
{
  if (LastClickedMenu)
  {
    LastClickedMenu->processLeftDrag((float)(e->x - 4), (float)(e->y - 4), (float)(e->xrel), (float)(e->yrel));
  }
  else
  {
    mGUIFrame->mouse_moved (e->x, e->y);
  }
}

void Menu::resizewindow()
{
  mGUIFrame->resize();
}

void Menu::loadMap(int mapID)
{
  delete gWorld;
  gWorld = nullptr;

  uidFixWindow->hide();

	for (DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it)
	{
		if (it->getInt(MapDB::MapID) == mapID)
		{
      gWorld = new World(it->getString(MapDB::InternalName));
      mGUICreditsWindow->hide();
      mGUIMinimapWindow->show();
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
              uidFixWindow->enterAt (pos);
            }
            mmw->deleteLater();
          }
        );

      return;
		}
	}

  LogError << "Map with ID " << mapID << " not found. Failed loading." << std::endl;
}

void Menu::loadBookmark(int bookmarkID)
{
  BookmarkEntry e = mBookmarks.at(bookmarkID);
  loadMap(e.mapID);
  enterMapAt(e.pos, e.av, e.ah);
}

void Menu::showSettings()
{
  mGUISettingsWindow->readInValues();
  mGUISettingsWindow->show();
}

void Menu::buildMenuBar()
{
  if (mGUImenuBar)
  {
    mGUIFrame->removeChild(mGUImenuBar);
    delete mGUImenuBar;
    mGUImenuBar = nullptr;
  }

  mGUImenuBar = new UIMenuBar();
  mGUImenuBar->AddMenu("File");
  mGUImenuBar->GetMenu("File")->AddMenuItemButton("Settings", [this] { showSettings(); });
  mGUImenuBar->GetMenu("File")->AddMenuItemButton("About", [this] { mGUICreditsWindow->show(); });
  mGUImenuBar->GetMenu("File")->AddMenuItemSwitch("exit ESC", &app.pop, true);
  mGUIFrame->addChild(mGUImenuBar);

  static const char* typeToName[] = { "Continent", "Dungeons", "Raid", "Battleground", "Arena" };
  static int nMapByType[] = { 0, 0, 0, 0, 0 };

  for (std::vector<MapEntry>::const_iterator it = mMaps.begin(); it != mMaps.end(); ++it)
  {
    nMapByType[it->areaType]++;
  }

  static const size_t nBookmarksPerMenu = 20;

  for (int i = 0; i < (sizeof(typeToName) / sizeof(*typeToName)); ++i)
  {
    mGUImenuBar->AddMenu(typeToName[i]);
    if (nMapByType[i] > nBookmarksPerMenu)
    {
      int nMenu = (nMapByType[i] / nBookmarksPerMenu) + 1;

      for (int j = 2; j <= nMenu; j++)
      {
        std::stringstream name;
        name << typeToName[i] << " (" << j << ")";
        mGUImenuBar->AddMenu(name.str());
      }
    }

    nMapByType[i] = 0;
  }


  for (std::vector<MapEntry>::const_iterator it = mMaps.begin(); it != mMaps.end(); ++it)
  {
    auto const map_id (it->mapID);
    if (nMapByType[it->areaType]++ < nBookmarksPerMenu)
    {
      mGUImenuBar->GetMenu(typeToName[it->areaType])->AddMenuItemButton(it->name, [map_id, this] { loadMap (map_id); });
    }
    else
    {
      std::stringstream name;
      name << typeToName[it->areaType] << " (" << (nMapByType[it->areaType] / nBookmarksPerMenu + 1) << ")";
      mGUImenuBar->GetMenu(name.str())->AddMenuItemButton(it->name, [map_id, this] { loadMap (map_id); });
    }
  }

  const size_t nBookmarkMenus = (mBookmarks.size() / nBookmarksPerMenu) + 1;

  if (mBookmarks.size())
  {
    mGUImenuBar->AddMenu("Bookmarks");
  }

  for (size_t i = 1; i < nBookmarkMenus; ++i)
  {
    std::stringstream name;
    name << "Bookmarks (" << (i + 1) << ")";
    mGUImenuBar->AddMenu(name.str());
  }

  int n = -1;
  for (std::vector<BookmarkEntry>::const_iterator it = mBookmarks.begin(); it != mBookmarks.end(); ++it)
  {
    std::stringstream name;
    const int page = (++n / nBookmarksPerMenu);
    if (page)
    {
      name << "Bookmarks (" << (page + 1) << ")";
    }
    else
    {
      name << "Bookmarks";
    }

    mGUImenuBar->GetMenu(name.str())->AddMenuItemButton(it->name, [n, this] { loadBookmark (n); });
  }
}

void Menu::createMapList()
{
  for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
  {
    MapEntry e;
    e.mapID = i->getInt(MapDB::MapID);
    e.name = i->getLocalizedString(MapDB::Name);
    e.areaType = i->getUInt(MapDB::AreaType);
    if (e.areaType == 3) e.name = i->getString(MapDB::InternalName);

    if (e.areaType < 0 || e.areaType > 4 || !World::IsEditableWorld(e.mapID))
      continue;

    mMaps.push_back(e);
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
