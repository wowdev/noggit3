#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

#include "appstate.h"
#include "vec3d.h"

// ui classes
class frame;
class statusBar;
class winCredits;
class minimapWindowUI;
class menuBar;

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
	Vec3D pos;
	float ah;
	float av;
};

class Menu : public AppState
{
private:
	frame* mGUIFrame;
	statusBar* mGUIStatusbar;
	winCredits* mGUICreditsWindow;
	minimapWindowUI* mGUIMinimapWindow;
	menuBar* mGUImenuBar;

	std::vector<MapEntry> mMaps;
	std::vector<BookmarkEntry> mBookmarks;

	Model* mBackgroundModel;
	int mLastBackgroundId;
	
	void createBookmarkList();
	void createMapList();
	void buildMenuBar();
	void randBackground();
	
	void resizewindow();

public:

	Menu();
	~Menu();

	void tick( float t, float dt );
	void display( float t, float dt );

	void keypressed( SDL_KeyboardEvent *e );
	void mouseclick( SDL_MouseButtonEvent *e );
	
	//! \todo Make private when new buttons are implemented.
	void loadMap( int mapID );
	void loadBookmark( int bookmarkID );
	
	//! \brief Enter the the map on the given location.
	void enterMapAt( Vec3D pos, bool autoHeight = true, float av = -30.0f, float ah = -90.0f );
};

#endif
