#ifndef __GRADIENT_H
#define __GRADIENT_H

#include "quaternion.h"
#include "frame.h"

class gradient:public frame
{
protected:  
  Vec4D  MinColor;
  Vec4D  MaxColor;
  Vec4D  ClickColor;
  float  value;
  void (*clickFunc)(float val);
public:
  bool  horiz;

  void  setValue(float f);
  void  setClickFunc(void (*f)(float val));
  void  setMinColor(float r, float g, float b, float a);
  void  setMaxColor(float r, float g, float b, float a);
  void  setClickColor(float r, float g, float b, float a);
  frame  *processLeftClick(float mx,float my);
  bool  processLeftDrag(float mx,float my, float xChange, float yChange);
  void  render() const;
};
#endif
