// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/UIWindow.h>

class UICloseWindow : public UIWindow
{
public:
	UICloseWindow(float x, float y, float w, float h, const std::string& pTitle, bool pMoveable = false);
};
