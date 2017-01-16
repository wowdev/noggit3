// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UICloseWindow.h>
#include <noggit/UITextBox.h>
#include <noggit/UIButton.h>
#include <noggit/UITreeView.h>
#include <noggit/UIModel.h>

class UIDoodadSpawner : public UICloseWindow
{
private:
	UITextBox::Ptr _tbox;
	UIButton::Ptr _button;
	UITreeView::Ptr _treeView;
	UIModel *modelView;

public:
	UIDoodadSpawner();

	void AddM2(const std::string& filename);
};
