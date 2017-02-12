// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class UITextBox;
class UICheckBox;
class UIFrame;

#include <noggit/ui/CloseWindow.h>

class UISettings : public UIWindow
{
public:
  UITextBox *gamePathField;
  UITextBox *wodPathField;
  UITextBox *projectPathField;
  UITextBox *farZField;
private:
  static const int winWidth = 500;
  static const int winHeight = 400;
    
  UICheckBox *tabletModeCheck;
  UICheckBox *autoselectCheck;
  UICheckBox *modelStatsCheck;
  UICheckBox *modelsBoxCheck;
  UICheckBox *randRotCheck;
  UICheckBox *randSizeCheck;
  UICheckBox *randTiltCheck;
    
public:
  UISettings();
  void readInValues();
  void resize();
};
