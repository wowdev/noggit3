#ifndef __MINIMAPWINDOWUI_H
#define __MINIMAPWINDOWUI_H

#include "UIWindow.h"

class Menu;
class World;

class UIMinimapWindow : public UIWindow
{
private:
  float borderwidth;
  float tilesize;
  float lookAt;
  Menu* mMenuLink;
  World* map;
  
public:
  explicit UIMinimapWindow( Menu* menuLink );
  explicit UIMinimapWindow( World* setMap );
  UIFrame* processLeftClick( float mx, float my );
  void resize();
  void render() const;
  void changePlayerLookAt( float ah );
};

#endif
