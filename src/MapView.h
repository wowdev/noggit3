#ifndef TEST_H
#define TEST_H

#include "appstate.h"

class frame;

class World;

enum eViewMode
{
	eViewMode_Help,
	eViewMode_Minimap,
	eViewMode_2D,
	eViewMode_3D
};
 
class MapView : public AppState, public HotKeyReceiver
{
	float ah,av,moving,strafing,updown,mousedir,movespd;
	bool key_w;
	bool look;
	bool hud;
	bool set_areaid;

  void save();


	float lastBrushUpdate;

	void doSelection(int selTyp);

	int mViewMode;

	void displayViewMode_Help( float t, float dt );
	void displayViewMode_Minimap( float t, float dt );
	void displayViewMode_2D( float t, float dt );
	void displayViewMode_3D( float t, float dt );

	float mTimespeed;

public:
  void quit();
	MapView(float ah0 = -90.0f, float av0 = -30.0f);
	~MapView();

	void tick(float t, float dt);
	void display(float t, float dt);

	void keypressed(SDL_KeyboardEvent *e);
	void mousemove(SDL_MouseMotionEvent *e);
	void mouseclick(SDL_MouseButtonEvent *e);
	void resizewindow();

	//! \todo	Remove when help is a window.
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
