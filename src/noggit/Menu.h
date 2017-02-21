// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/AppState.h>

#include <boost/optional.hpp>

#include <memory>
#include <string>
#include <vector>

// ui classes
class UIFrame;
class UIStatusBar;
class UIAbout;
class UISettings;
class UIMinimapWindow;
class UIMenuBar;
namespace ui
{
  class uid_fix_window;
}


class World;
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

  void display(float t, float dt);

  virtual void mouseReleaseEvent (SDL_MouseButtonEvent*) override;
  virtual void mousePressEvent (SDL_MouseButtonEvent*) override;

  void mousemove(SDL_MouseMotionEvent* e);

  //! \todo Make private when new buttons are implemented.
  void loadMap(int mapID);
  void loadBookmark(int bookmarkID);

  //! \brief Enter the the map on the given location.
  void enterMapAt(math::vector_3d pos, float av = -30.0f, float ah = -90.0f);

  ui::uid_fix_window* uidFixWindow;

private:
  std::unique_ptr<UIFrame> mGUIFrame;
  UIAbout* mGUICreditsWindow;
  UISettings* mGUISettingsWindow;
  UIMinimapWindow* mGUIMinimapWindow;
  UIMenuBar* mGUImenuBar;

  std::vector<MapEntry> mMaps;
  std::vector<BookmarkEntry> mBookmarks;

  void createBookmarkList();
  void createMapList();
  void buildMenuBar();
  void showSettings();

  void resizewindow();
};
