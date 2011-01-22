#ifndef __MINIMAPWINDOWUI_H
#define __MINIMAPWINDOWUI_H

#include "window.h"

class Menu;
class World;

class minimapWindowUI : public window
{
private:
	float tilesize;
	float borderwidth;
	float lookAt;
	Menu* mMenuLink;
	World* map;
public:
	minimapWindowUI( Menu* menuLink );
	minimapWindowUI( World *setMap );
	frame *processLeftClick( float mx, float my );
	void resize();
	void render() const;
	void changePlayerLookAt(float ah);
};

#endif
