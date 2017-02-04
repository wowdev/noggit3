// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/ui/Window.h>
#include <noggit/tool_enums.hpp>

class UIMapViewGUI;
class UIText;
class UIToolbarIcon;

class UIToolbar : public UIWindow
{
private:
  void SetIcon(editing_mode pIcon, const std::string& pIconFile);

  std::vector<UIToolbarIcon*> mToolbarIcons;
  UIText* text;
  // current selected Icon
  editing_mode selectedIcon;

public:
  UIToolbar(float x, float y);
  void IconSelect(editing_mode);
};
