// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/UIWindow.h>

class UIText;

class UIStatusBar : public UIWindow
{
private:
	UIText* leftInfo;
	UIText* rightInfo;

public:
	UIStatusBar(float x, float y, float width, float height);
	void render() const;
	void resize();
	void setLeftInfo(const std::string& pText);
	void setRightInfo(const std::string& pText);
};
