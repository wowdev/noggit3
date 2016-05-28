// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/UIEventClasses.h>
#include <noggit/UIWindow.h>

class UIMapViewGUI;
class UIText;
class UIToolbarIcon;
class UITexture;

class UIToolbar : public UIWindow, public UIEventListener
{
private:
  UIMapViewGUI* mainGui;
  void SetIcon( int pIcon, const std::string& pIconFile );

public:
  UIToolbarIcon* mToolbarIcons[10];
  UIText* text;
  // current selected Icon
  int selectedIcon;

  // current active texture
  UITexture* current_texture;
  UIToolbar(float x, float y, UIMapViewGUI *setGui);
  void set_icon_visual (int i);
  void IconSelect(int i);
};
