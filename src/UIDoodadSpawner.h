#ifndef UIDOODADSPAWNER_H
#define UIDOODADSPAWNER_H

#include "UICloseWindow.h"
#include "UITextBox.h"
#include "UIButton.h"
#include "UITreeView.h"

class World;

class UIDoodadSpawner : public UICloseWindow
{
public:
  UIDoodadSpawner (World*);

  void AddM2( const std::string& filename );

private:
  UITextBox::Ptr _tbox;
  UIButton::Ptr _button;
  UITreeView::Ptr _treeView;

  World* _world;
};

#endif
