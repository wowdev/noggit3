// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/AppState.h>
#include <noggit/ModelManager.h>

#include <boost/optional.hpp>

#include <string>
#include <vector>

// ui classes
class UIFrame;
class UIStatusBar;
class UIAbout;
class UIMinimapWindow;
class UIMenuBar;
namespace ui
{
  class uid_fix_window;
}


class World;
class Model;
class MapView;

struct MapEntry
{
  int mapID;
  std::string name;
  int areaType;
};

struct BookmarkEntry
{
  int mapID;
  std::string name;
  math::vector_3d pos;
  float ah;
  float av;
};

class Menu : public AppState
{
public:
  Menu();
  ~Menu();

  void tick(float t, float dt);
  void display(float t, float dt);

  void mouseclick(SDL_MouseButtonEvent* e);
  void mousemove(SDL_MouseMotionEvent* e);

  //! \todo Make private when new buttons are implemented.
  void loadMap(int mapID);
  void loadBookmark(int bookmarkID);

  //! \brief Enter the the map on the given location.
  void enterMapAt(math::vector_3d pos, bool pAutoHeight = true, float av = -30.0f, float ah = -90.0f);

  ui::uid_fix_window* uidFixWindow;

private:
  UIFrame* mGUIFrame;
  UIStatusBar* mGUIStatusbar;
  UIAbout* mGUICreditsWindow;
  UIMinimapWindow* mGUIMinimapWindow;
  UIMenuBar* mGUImenuBar;

  std::vector<MapEntry> mMaps;
  std::vector<BookmarkEntry> mBookmarks;

  boost::optional<scoped_model_reference> mBackgroundModel;
  int mLastBackgroundId;

  void createBookmarkList();
  void createMapList();
  void buildMenuBar();
  void randBackground();

  void resizewindow();
};
