// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UICloseWindow.h>
#include <noggit/MapView.h>

class UIExitWarning : public UICloseWindow
{
private:
	static const int winWidth = 320;
	static const int winHeight = 120;
	MapView *_MapView;
public:
	UIExitWarning(MapView *mapView);
	void resize();
	void exitNow();
};
