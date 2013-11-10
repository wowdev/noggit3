#ifndef UIDOODADSPAWNER_H
#define UIDOODADSPAWNER_H

#include "UICloseWindow.h"
#include "UITextBox.h"
#include "UIButton.h"
#include "UITreeView.h"
#include "UIModel.h"

class UIDoodadSpawner : public UICloseWindow
{
private:
  UITextBox::Ptr _tbox;
  UIButton::Ptr _button;
  UITreeView::Ptr _treeView;
  UIModel *modelView;

public:
  UIDoodadSpawner();

  void AddM2( const std::string& filename );
};

#endif
