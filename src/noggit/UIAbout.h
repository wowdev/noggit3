// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "UICloseWindow.h"

class UIAbout : public UIWindow
{
private:
	static const int winWidth = 400;
	static const int winHeight = 230;

public:
	UIAbout();
	void resize();
};
