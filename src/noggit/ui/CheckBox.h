// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Frame.h>

#include <noggit/ui/Texture.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/ToggleGroup.h>

#include <string>
#include <functional>

class UICheckBox : public UIFrame
{
public:
  typedef UICheckBox* Ptr;
  typedef std::function<void(bool, int)> ClickFunction;

protected:
  UITexture::Ptr check;
  UIText::Ptr text;
  bool checked;
  int id;
  ClickFunction clickFunc;

  UIToggleGroup::Ptr mToggleGroup;

public:
  UICheckBox(float, float, const std::string&);
  UICheckBox(float, float, const std::string&, UIToggleGroup *, int);
  UICheckBox(float xPos, float yPos, const std::string& pText, ClickFunction function, int pClickFuncParameter);
  UICheckBox (float x, float y, std::string const& name, bool* value);
  void SetToggleGroup(UIToggleGroup *, int);
  void setText(const std::string&);
  void setState(bool);
  bool getState();
  void setClickFunc(void(*f)(bool, int), int);

  UIFrame *processLeftClick(float, float);
};
