// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

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
  math::degrees lookAt;
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
	void changePlayerLookAt(math::degrees ah);
};
