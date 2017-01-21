// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/ui/Window.h>

class UIMapViewGUI;
class UITexture;

class UICurrentTexture : public UIWindow
{
private:
  UIMapViewGUI* mainGui;

public:

  // current active texture
  UITexture* current_texture;
  UICurrentTexture(float x, float y, UIMapViewGUI *setGui);
  void IconSelect(int i);
};
