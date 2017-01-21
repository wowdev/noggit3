// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>
#include <noggit/ui/TextBox.h>
#include <noggit/ui/Button.h>
#include <noggit/ui/TreeView.h>
#include <noggit/ui/Model.h>

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
