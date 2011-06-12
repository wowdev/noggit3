#ifndef __MINIMIZEBUTTON_H
#define __MINIMIZEBUTTON_H

#include "buttonUI.h"

class frame;

class MinimizeButton : public buttonUI
{
private:
  frame * mParent;
public:
  MinimizeButton( float pWidth, frame * pParent );

  frame *processLeftClick( float mx, float my );
};
#endif
