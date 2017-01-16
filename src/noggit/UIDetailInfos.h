// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/UIWindow.h>

class detailInfos;
class UIMapViewGUI;
class UIText;

class UIDetailInfos : public UIWindow
{
private:
	UIMapViewGUI* mainGui;
	UIText* theInfos;

public:
	UIDetailInfos(float x, float y, float width, float height, UIMapViewGUI *setGui);
	void setText(const std::string& t);
};
