#ifndef __UI_EXITWARNING_H
#define __UI_EXITWARNING_H

#include "UICloseWindow.h"
#include "MapView.h"

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


#endif