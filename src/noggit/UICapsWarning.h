// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UIWindow.h>

class UICapsWarning : public UIWindow
{
private:
	static const int winWidth = 320;
	static const int winHeight = 80;
public:
	UICapsWarning();
	void resize();
};
