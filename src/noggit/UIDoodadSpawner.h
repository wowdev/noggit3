// UIDoodadSpawner.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>

#ifndef UIDOODADSPAWNER_H
#define UIDOODADSPAWNER_H

#include <noggit/UICloseWindow.h>
#include <noggit/UITextBox.h>
#include <noggit/UIButton.h>
#include <noggit/UITreeView.h>

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
