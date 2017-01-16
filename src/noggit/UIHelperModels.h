// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UICloseWindow.h>
#include <noggit/MapView.h>

class UIHelperModels : public UICloseWindow
{
private:
	static const int winWidth = 310;
	static const int winHeight = 230;


public:
	UIHelperModels(MapView *mapview);
	void resize();
	void addModelNow(int model);
	MapView *_mapView;
};
