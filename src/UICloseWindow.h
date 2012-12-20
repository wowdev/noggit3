#ifndef __CLOSEWINDOWUI_H
#define __CLOSEWINDOWUI_H

#include <string>

#include "UIWindow.h"

class UICloseWindow : public UIWindow
{
public:
  UICloseWindow( float x, float y, float w, float h, const std::string& pTitle, bool pMoveable = false );
};

#endif
