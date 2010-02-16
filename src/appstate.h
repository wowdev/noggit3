#ifndef APPSTATE_H
#define APPSTATE_H

#include <SDL/SDL.h>

class AppState {
public:
	AppState() {};
	virtual ~AppState() {};

	virtual void tick(float t, float dt) {};
	virtual void display(float t, float dt) {};

	virtual void mousemove(SDL_MouseMotionEvent *e) {};
	virtual void mouseclick(SDL_MouseButtonEvent *e) {};
	virtual void keypressed(SDL_KeyboardEvent *e) {};
	virtual void resizewindow() {};

	//virtual void onPop() {};
};

#endif