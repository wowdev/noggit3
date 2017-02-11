// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class UITextBox;

#include <noggit/ui/CloseWindow.h>

class UISettings : public UIWindow
{
private:
  static const int winWidth = 500;
  static const int winHeight = 400;
    
  UITextBox *gamePathField;
  UITextBox *wodPathField;
  UITextBox *projectPathField;
  UITextBox *farZField;

public:
  UISettings();
  void resize();
};
