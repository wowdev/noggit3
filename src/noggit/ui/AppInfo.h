// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/ui/CloseWindow.h>
#include <noggit/ui/Text.h>

class UIMapViewGUI;

class UIAppInfo : public UICloseWindow
{
public:
  typedef UIAppInfo* Ptr;

private:
  UIText::Ptr theInfos;
  std::string mModelToLoad;

public:
  UIAppInfo(float x, float y, float width, float height);
  void setText(const std::string& t);
};
