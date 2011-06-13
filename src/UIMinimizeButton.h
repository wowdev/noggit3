#ifndef __MINIMIZEBUTTON_H
#define __MINIMIZEBUTTON_H

#include "UIButton.h"

class UIFrame;

class UIMinimizeButton : public UIButton
{
public:
  UIMinimizeButton( float pWidth, UIFrame * pParent );

  UIFrame *processLeftClick( float mx, float my );
};
#endif
