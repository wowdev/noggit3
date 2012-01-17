// UIGradient.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#ifndef __GRADIENT_H
#define __GRADIENT_H

#include <noggit/Quaternion.h> // Vec4D
#include <noggit/UIFrame.h> // UIFrame

class UIGradient : public UIFrame
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
  UIFrame  *processLeftClick(float mx,float my);
  bool  processLeftDrag(float mx,float my, float xChange, float yChange);
  void  render() const;
};
#endif
