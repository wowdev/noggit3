// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Button.h>

class UIMinimizeButton : public UIButton
{
  std::function<void()> _on_hide;
public:
  typedef UIMinimizeButton* Ptr;

  UIMinimizeButton(float pWidth, std::function<void()> on_hide = []{});

  UIFrame::Ptr processLeftClick(float mx, float my);
};
