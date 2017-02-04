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

  std::function<void (editing_mode)> _set_editing_mode;

public:
  UIToolbar(float x, float y, std::function<void (editing_mode)> set_editing_mode);
  void IconSelect(editing_mode);
};
