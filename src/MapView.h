#ifndef TEST_H
#define TEST_H

#include "appstate.h"
#include "ui.h"

class World;

enum eViewMode
{
	eViewMode_Help,
	eViewMode_Minimap,
	eViewMode_2D,
	eViewMode_3D
};
 
class MapView :public AppState
{
	GLuint tex;

	float ah,av,moving,strafing,updown,mousedir,movespd;
	bool look;
	bool hud;

	World *world;

	frame tileFrames;

	float lastBrushUpdate;

	void doSelection(int selTyp);

	int mViewMode;

	void displayViewMode_Help( float t, float dt );
	void displayViewMode_Minimap( float t, float dt );
	void displayViewMode_2D( float t, float dt );
	void displayViewMode_3D( float t, float dt );

	float mTimespeed;

public:

	MapView(World *w, float ah0 = -90.0f, float av0 = -30.0f);
	~MapView();

	void tick(float t, float dt);
	void display(float t, float dt);

	void keypressed(SDL_KeyboardEvent *e);
	void mousemove(SDL_MouseMotionEvent *e);
	void mouseclick(SDL_MouseButtonEvent *e);
	void resizewindow();

	//! \todo  Remove when help is a window.
	void ViewHelp()
	{
		mViewMode = eViewMode_Help;
	}
	void View3D()
	{
		mViewMode = eViewMode_3D;
	}

};


#endif
