#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "appstate.h"
#include "wowmapview.h"
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
};

struct Bookmark : public Clickable 
{
	std::string basename, name, label;
	Vec3D pos;
	float ah, av;
};

enum Commands {
	CMD_SELECT,
	CMD_LOAD_WORLD,
	CMD_DO_LOAD_WORLD,
	CMD_BACK_TO_MENU,
};

class Menu : public AppState
{
	int sel, cmd, x, y, cz, cx;

	int minimap_x, minimap_y;
	minimapWindowUI *minimap_win;
	World *world;

	std::vector<MapEntry> maps;
	std::vector<Bookmark> bookmarks;

	bool setpos;
	float ah,av;

	boost::shared_ptr<Model> bg;
	GLuint loading;
	float mt;

	int lastbg;

public:
	Menu();

	void tick(float t, float dt);
	void display(float t, float dt);

	void keypressed(SDL_KeyboardEvent *e);
	void mousemove(SDL_MouseMotionEvent *e);
	void mouseclick(SDL_MouseButtonEvent *e);

	void refreshBookmarks();
	void randBackground();
};


#endif
