#ifndef __SCROLLBARUI_H
#define __SCROLLBARUI_H

#include "UIFrame.h"

class UITexture;

class UIScrollBar : public UIFrame
{
protected:
  int* mTarget;
  int num;
  int value;
  void ( *changeFunc )( UIFrame * sender, int value );
  UITexture* ScrollKnob;
  
public:
  int* extValue;
  void clickReturn(int v);
  UIScrollBar(float xpos, float ypos, float height, int num);
  int getValue() const;
  void setValue(int i);
  void setNum(int i);
  bool processLeftDrag(float mx,float my, float xChange, float yChange);
  UIFrame* processLeftClick(float mx,float my);
  void setChangeFunc( void (*f)( UIFrame * sender, int value));
  void setScrollNoob( );
};
#endif
