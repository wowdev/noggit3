#ifndef __UI_WATERSAVEWARNING_H
#define __UI_WATERSAVEWARNING_H

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


#endif