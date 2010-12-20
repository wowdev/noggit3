#ifndef __MINIMAPWINDOWUI_H
#define __MINIMAPWINDOWUI_H

#include "window.h"

class Menu;

class minimapWindowUI : public window
{
private:
	float tilesize;
	float borderwidth;
	Menu* mMenuLink;
public:
	minimapWindowUI( Menu* menuLink );
	frame *processLeftClick( float mx, float my );
	void resize();
	void render() const;
};

#endif
