#include "UIGradient.h"

#include "Video.h" // gl*

void UIGradient::render() const
{
  if(hidden)
    return;
  if(horiz)
  {
    glBegin(GL_TRIANGLE_STRIP);
    glColor4fv(&MinColor.x);
    glVertex2f(x,y);    
    glVertex2f(x,y+height);
    glColor4fv(&MaxColor.x);
    glVertex2f(x+width,y);
    glVertex2f(x+width,y+height);
    glEnd();
    if(clickable)
    {
      glColor4fv(&ClickColor.x);
      glBegin(GL_LINE);
      glVertex2f(x+width*value,y);
      glVertex2f(x+width*value,y+height);
      glEnd();
    }
  }
  else
  {
    glBegin(GL_TRIANGLE_STRIP);
    glColor4fv(&MaxColor.x);    
    glVertex2f(x+width,y);    
    glVertex2f(x,y);
    glColor4fv(&MinColor.x);
    glVertex2f(x+width,y+height);
    glVertex2f(x,y+height);
    glEnd();

    if(clickable)
    {
      glColor4fv(&ClickColor.x);
      glBegin(GL_TRIANGLE_STRIP);
      glVertex2f(x,y+height*value-1);
      glVertex2f(x+width,y+height*value-1);
      glVertex2f(x,y+height*value+1);
      glVertex2f(x+width,y+height*value+1);
      glEnd();
    }
  }
}

void UIGradient::setMaxColor(float r,float g, float b,float a)
{
  MaxColor=Vec4D(r,g,b,a);
}

void UIGradient::setMinColor(float r,float g, float b,float a)
{
  MinColor=Vec4D(r,g,b,a);
}

void UIGradient::setClickColor(float r,float g, float b,float a)
{
  ClickColor=Vec4D(r,g,b,a);
}

void UIGradient::setClickFunc(void (*f)(float val))
{
  value=0.0f;
  clickFunc=f;
  clickable=true;
}

UIFrame *UIGradient::processLeftClick(float mx,float my)
{
  if(clickable)
  {
    if(horiz)
      value=mx/width;
    else
      value=my/height;
    if(value<0.0f)
      value=0.0f;
    else if(value>1.0f)
      value=1.0f;
    clickFunc(value);
    return this;
  }
  else
    return 0;
}

bool UIGradient::processLeftDrag(float mx,float my, float xDrag, float yDrag)
{
  float tx,ty;
  parent->getOffset(&tx,&ty);
  mx-=tx;
  my-=ty;
  if(clickable)
  {
    if(horiz)
      value=mx/width;
    else
      value=my/height;
    if(value<0.0f)
      value=0.0f;
    else if(value>1.0f)
      value=1.0f;
    clickFunc(value);
    return true;
  }

  if(movable)
  {
    x+=xDrag;
    y+=yDrag;
    return true;
  }

  return false;
}

void UIGradient::setValue(float f)
{
  value=f;
  if(value<0.0f)
    value=0.0f;
  else if(value>1.0f)
    value=1.0f;
  clickFunc(value);
}
