#ifndef __MINIMIZEBUTTON_H
#define __MINIMIZEBUTTON_H

#include "UIButton.h"

class UIMinimizeButton : public UIButton
{
public:
  typedef UIMinimizeButton* Ptr;

  UIMinimizeButton( float pWidth );

  UIFrame::Ptr processLeftClick( float mx, float my );
};
#endif
