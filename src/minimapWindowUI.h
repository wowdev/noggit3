#ifndef __MINIMAPWINDOWUI_H
#define __MINIMAPWINDOWUI_H

#include "window.h"

class Menu;

class minimapWindowUI : public window
{
private:
  static const float tilesize = 12.0f;
  Menu* mMenuLink;
public:
	minimapWindowUI( Menu* menuLink, float x, float y);
	frame *processLeftClick( float mx, float my );
	void render() const;
};

#endif
