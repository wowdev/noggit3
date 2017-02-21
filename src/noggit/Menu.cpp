// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <noggit/MapView.h>
#include <noggit/Menu.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Settings.h>
#include <noggit/Settings.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMOInstance.h> // WMOInstance (only for loading WMO only maps, we never load..)
#include <noggit/WMOInstance.h> // WMOInstance (only for loading WMO only maps, we never load..)
#include <noggit/World.h>
#include <noggit/World.h>
#include <noggit/application.h> // fonts, APP_*
#include <noggit/map_index.hpp>
#include <noggit/map_index.hpp>
#include <noggit/ui/About.h> // UIAbout
#include <noggit/ui/SettingsPanel.h> //UISettings
#include <noggit/ui/Frame.h> // UIFrame
#include <noggit/ui/MenuBar.h> // UIMenuBar, menu items, ..
#include <noggit/ui/MinimapWindow.h> // UIMinimapWindow
#include <noggit/ui/StatusBar.h> // UIStatusBar
#include <noggit/ui/uid_fix_window.hpp>

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
	, mBackgroundModel(boost::none)
	, mLastBackgroundId(-1)
  , uidFixWindow(nullptr)
{
  gWorld = nullptr;

  mGUIFrame = std::make_unique<UIFrame> (0.0f, 0.0f, (float)video.xres(), (float)video.yres());
  mGUIMinimapWindow = new UIMinimapWindow(this);
  mGUIMinimapWindow->hide();
  mGUIFrame->addChild(mGUIMinimapWindow);
  mGUICreditsWindow = new UIAbout();
  mGUIFrame->addChild(mGUICreditsWindow);
  mGUISettingsWindow = new UISettings();

  uidFixWindow = new ui::uid_fix_window(this);
  uidFixWindow->hide();
  mGUIFrame->addChild(uidFixWindow);


	createMapList();
	createBookmarkList();
	buildMenuBar();
	randBackground();

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

//! \todo Add TBC and WOTLK.
//! \todo Use std::array / boost::array.
//const std::string uiModels[] = { "BloodElf", "Deathknight", "Draenei", "Dwarf", "Human", "MainMenu", "NightElf", "Orc", "Scourge", "Tauren" };
//Steff: Turn of the ugly once
const std::string uiModels[] = { "Deathknight", "Draenei", "Dwarf", "MainMenu", "NightElf", "Orc" };


std::string buildModelPath(size_t index)
{
  assert(index < sizeof(uiModels) / sizeof(const std::string));

  return "Interface\\Glues\\Models\\UI_" + uiModels[index] + "\\UI_" + uiModels[index] + ".m2";
}

Menu::~Menu()
{
  delete gWorld;
  gWorld = nullptr;
}

void Menu::randBackground()
{
  mBackgroundModel.reset();

  int randnum;
  do
  {
    randnum = misc::randint(0, sizeof(uiModels) / sizeof(const std::string) - 1);
  } while (randnum == mLastBackgroundId);

  mLastBackgroundId = randnum;

  mBackgroundModel = scoped_model_reference (buildModelPath(randnum));
  mBackgroundModel.get()->mPerInstanceAnimation = true;
}


void Menu::enterMapAt(math::vector_3d pos, float av, float ah)
{
  video.farclip((const float)Settings::getInstance()->FarZ);

  gWorld->camera = math::vector_3d(pos.x, pos.y, pos.z);

  gWorld->initDisplay();
  gWorld->mapIndex.enterTile(tile_index(pos));

  app.getStates().push_back(new MapView(ah, av, math::vector_3d(pos.x, pos.y, pos.z - 1.0f))); // on gPop, MapView is deleted.

  mGUIMinimapWindow->hide();

  mBackgroundModel.reset();
}

void Menu::tick(float t, float /*dt*/)
{
  globalTime = (int)(t * 1000.0f);

  if (mBackgroundModel)
  {
    mBackgroundModel.get()->updateEmitters(t);
  }
  else
  {
    randBackground();
  }
}

void Menu::display(float /*t*/, float /*dt*/)
{
  // 3D: Background.
  gl.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  video.set3D();

  gl.disable(GL_FOG);

  gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);

  math::vector_4d la(0.1f, 0.1f, 0.1f, 1.0f);
  gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, la);

  gl.enable(GL_COLOR_MATERIAL);
  gl.colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);
  for (opengl::light light = GL_LIGHT0; light < GL_LIGHT0 + 8; ++light)
  {
    gl.lightf(light, GL_CONSTANT_ATTENUATION, 0.0f);
    gl.lightf(light, GL_LINEAR_ATTENUATION, 0.7f);
    gl.lightf(light, GL_QUADRATIC_ATTENUATION, 0.03f);
    gl.disable(light);
  }

  gl.enable(GL_CULL_FACE);
  gl.enable(GL_DEPTH_TEST);
  gl.depthFunc(GL_LEQUAL);
  gl.enable(GL_LIGHTING);
  opengl::texture::enable_texture();

  mBackgroundModel.get()->cam->setup(globalTime);
  mBackgroundModel.get()->draw();

  opengl::texture::disable_texture();
  gl.disable(GL_LIGHTING);
  gl.disable(GL_DEPTH_TEST);
  gl.disable(GL_CULL_FACE);

  // 2D: UI.

  video.set2D();
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
