#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

#include "appstate.h"
#include "noggit.h"
#include "vec3d.h"
#include "ui.h"

class World;
class Model;

struct Clickable 
{
	int x0, y0, x1, y1;

	bool hit( int x, int y );
};

struct MapEntry : public Clickable 
{
	std::string name, description;
	int AreaType, IsBattleground, loadingscreen;
	freetype::font_data font;
	int mid;
};

struct Bookmark : public Clickable 
{
	std::string basename, name, label;
	Vec3D pos;
	float ah, av;
	int mid;
};

enum Commands {
	CMD_SELECT,
	CMD_LOAD_WORLD,
	CMD_DO_LOAD_WORLD,
	CMD_BACK_TO_MENU
};

class Menu : public AppState
{
	int sel, newsel,newbookmark, cmd, x, y, cz, cx;

	// frame to place all gui elemnts on
	frame guiFrame;

	// status and menu bar
	statusBar	*guiStatusbar;
	menuBar		*mbar;

	winCredits *mCredits;

	int minimap_x, minimap_y;
	minimapWindowUI *minimap_win;
	World *world;

	std::vector<MapEntry> maps;
	std::vector<Bookmark> bookmarks;

	bool setpos;
	float ah,av;

	Model *bg;
	GLuint loading;
	float mt;

	int lastbg;

public:
	Menu();
	~Menu();

	void tick(float t, float dt);
	void display(float t, float dt);

	void keypressed(SDL_KeyboardEvent *e);
	void mousemove(SDL_MouseMotionEvent *e);
	void mouseclick(SDL_MouseButtonEvent *e);

	void refreshBookmarks();
	void randBackground();
	
	void resizewindow( );
	void loadMap( int mid );
	void loadBookmark( int mid );
};


#endif
