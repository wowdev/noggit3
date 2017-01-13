#ifndef __HELPER_MODELS_H
#define __HELPER_MODELS_H

#include "UICloseWindow.h"
#include "MapView.h"

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

#endif
