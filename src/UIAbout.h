#ifndef __WIN_CREDITS_H
#define __WIN_CREDITS_H

#include "UICloseWindow.h"

class UIAbout : public UICloseWindow
{
private:
  static const int winWidth = 400;
  static const int winHeight = 230;
  
public:
  UIAbout();
  void resize();
};

#endif
