#ifndef __TOOLBAR_H
#define __TOOLBAR_H

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
  void IconSelect(int i);
};
#endif

