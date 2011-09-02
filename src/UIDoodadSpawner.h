#ifndef UIDOODADSPAWNER_H
#define UIDOODADSPAWNER_H

#include "UICloseWindow.h"

class UITextBox;
class UIButton;

class UIDoodadSpawner : public UICloseWindow
{
public:
  UIDoodadSpawner();
  
  void AddM2();

  UITextBox* _tbox;
  UIButton* _button;
};

#endif
