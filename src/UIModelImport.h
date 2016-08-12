#ifndef __MODEL_IMPORT_H
#define __MODEL_IMPORT_H

#include "UICloseWindow.h"
#include "UITextBox.h"
#include "MapView.h"

class UIListView;

class UIModelImport : public UICloseWindow
{
private:
	static const int winWidth = 440;
	static const int winHeight = 270;
	MapView *_mapView;
	UIListView* MoldelList;
  UITextBox* _textBox;

public:
	UIModelImport(MapView *mapview);
	void resize();
	void builModelList();
	void addTXTModel(int id);
};

#endif
