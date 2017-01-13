// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "UIWindow.h"
#include "MapView.h"

class UIWaterSaveWarning : public UIWindow
{
private:
	static const int winWidth = 320;
	static const int winHeight = 80;
	MapView *_MapView;
public:
	UIWaterSaveWarning(MapView *mapView);
	void resize();
};
