#ifndef __CURSORSWITCHER_H
#define __CURSORSWITCHER_H

#include "UICloseWindow.h"

class UICursorSwitcher : public UICloseWindow
{
public:
  UICursorSwitcher(float x, float y, float w, float h);

private:
  float xPos, zPos;
};

#endif