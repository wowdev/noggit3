#ifndef __MINIMAPWINDOWUI_H
#define __MINIMAPWINDOWUI_H

#include "UIWindow.h"
#include "UIText.h"

class Menu;
class World;


class UIMinimapWindow : public UIWindow
{
private:
	float borderwidth;
	float bottomspace;
	float tilesize;
	float lookAt;
	Menu* mMenuLink;
	World* map;
	UIText::Ptr cursor_position;

public:
	explicit UIMinimapWindow(Menu* menuLink);
	explicit UIMinimapWindow(World* setMap);
	UIFrame* processLeftClick(float mx, float my);
	void mousemove(SDL_MouseMotionEvent *e);
	void resize();
	void render() const;
	void changePlayerLookAt(float ah);
};

#endif
