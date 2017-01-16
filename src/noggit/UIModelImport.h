// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UICloseWindow.h>
#include <noggit/UITextBox.h>
#include <noggit/MapView.h>

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
