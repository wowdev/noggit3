#ifndef __MODEL_IMPORT_H
#define __MODEL_IMPORT_H

#include "UICloseWindow.h"
#include "MapView.h"
class UIListView;

class UIModelImport : public UICloseWindow
{
private:
	static const int winWidth = 440;
	static const int winHeight = 240;
	MapView *_mapView;
	UIListView* MoldelList;

public:
	UIModelImport(MapView *mapview);
	void resize();
	void builModelList();
	void addTXTModel(int id);
};

#endif
