// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/ui/EventClasses.h>
#include <noggit/ui/Window.h>

class UIMapViewGUI;
class UIText;
class UIToolbarIcon;

class UIToolbar : public UIWindow, public UIEventListener
{
private:
  UIMapViewGUI* mainGui;
  void SetIcon(int pIcon, const std::string& pIconFile);

public:
  UIToolbarIcon* mToolbarIcons[10];
  UIText* text;
  // current selected Icon
  int selectedIcon;

  UIToolbar(float x, float y, UIMapViewGUI *setGui);
  void IconSelect(int i);
};
