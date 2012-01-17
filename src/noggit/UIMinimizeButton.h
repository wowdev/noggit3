// UIMinimizeButton.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#ifndef __MINIMIZEBUTTON_H
#define __MINIMIZEBUTTON_H

#include <noggit/UIButton.h>

class UIMinimizeButton : public UIButton
{
public:
  typedef UIMinimizeButton* Ptr;

  UIMinimizeButton( float pWidth );

  UIFrame::Ptr processLeftClick( float mx, float my );
};
#endif
