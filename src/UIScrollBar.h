#ifndef __SCROLLBARUI_H
#define __SCROLLBARUI_H

#include "UIFrame.h"
#include "UITexture.h"

class UIScrollBar : public UIFrame
{
public:
  typedef UIScrollBar* Ptr;

  static const float WIDTH;

  enum Orientation
  {
    Horizontal,
    Vertical,
  };

protected:
  int* mTarget;
  int num;
  int value;
  void ( *changeFunc )( UIFrame::Ptr sender, int value );
  UITexture::Ptr ScrollKnob;
  Orientation _orientation;

public:
  UIScrollBar( float x, float y, float height, int num, Orientation orientation = Vertical);

  int* extValue;
  void clickReturn(int v);
  int getValue() const;
  void setValue(int i);
  void setNum(int i);
  bool processLeftDrag(float mx,float my, float xChange, float yChange);
  UIFrame::Ptr processLeftClick(float mx,float my);
  void setChangeFunc( void (*f)( UIFrame::Ptr sender, int value));
  void setScrollNoob( );
};
#endif
