#include "UIScrollbar.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "Log.h"
#include "Misc.h"
#include "Noggit.h" // arial14, arialn13
#include "UIButton.h"
#include "UIText.h"
#include "UITexture.h"

void scrollbarProcessClick(UIFrame *f,int id)
{
  (reinterpret_cast<UIScrollBar*>(f->parent))->clickReturn(id);
}

UIScrollBar::UIScrollBar(float xpos, float ypos, float h, int n)
{
  x=xpos;
  y=ypos;
  width=16;
  height=h;
  
  UIButton* ScrollUp=new UIButton(-6.0f,-8.0f,32.0f,32.0f,"Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Up.blp","Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Down.blp");
  ScrollUp->setClickFunc(scrollbarProcessClick,0);
  addChild(ScrollUp);

  UIButton* ScrollDown=new UIButton(-6.0f,height-24.0f,32.0f,32.0f,"Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Up.blp","Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Down.blp");
  ScrollDown->setClickFunc(scrollbarProcessClick,1);
  addChild(ScrollDown);
  ScrollKnob=new UITexture(-6.0f,10.0f,32.0f,32.0f,"Interface\\Buttons\\UI-ScrollBar-Knob.blp");
  addChild(ScrollKnob);
  value=0;
  num=n;
  changeFunc=0;
}

bool UIScrollBar::processLeftDrag(float /*mx*/,float my, float /*xChange*/, float /*yChange*/)
{
  if(num<0) return false;
  float tx,ty;
  this->getOffset(&tx,&ty);
  my-=(ty+32);
  value=misc::FtoIround(num*my/(height-64));

  if(value<0)
    value=0;
  else if(value>num-1)
    value=num-1;

  if(num>0)
    setScrollNoob();

  if(changeFunc)
    changeFunc(this,value);
  return true;
}

UIFrame* UIScrollBar::processLeftClick(float mx,float my)
{
  if(num<0) return this;
  UIFrame * lTemp;
  for( std::vector<UIFrame*>::reverse_iterator child = children.rbegin(); child != children.rend(); child++ )
  {
    if( !( *child )->hidden && ( *child )->IsHit( mx, my ) )
    {
      lTemp = ( *child )->processLeftClick( mx - ( *child )->x, my - ( *child )->y );
      if( lTemp )
        return lTemp;
    }
  }
  return this;
}

void UIScrollBar::clickReturn(int id)
{
  if(num<0) return;
  if(id==0)//Scroll Up
  {
    if(value!=0)
      value--;
  }
  else if(id==1)//Scroll Down
  {
    if(value!=num-1)
      value++;
  }
  else//ScrollKnob
  {
    // handeld with processLeftClick and processLeftDrag
  }
  
  //Update ScrollKnob Position
  setScrollNoob();

  // call changeFunc if set
  if(changeFunc)
    changeFunc(this,value);
}

void UIScrollBar::setChangeFunc( void (*f)( UIFrame *, int ))
{
  changeFunc=f;
}

int UIScrollBar::getValue() const
{
  return value;
}

void UIScrollBar::setValue(int i)
{
  if(i>-1 && i < num)
  value=i;
  setScrollNoob();
}

void UIScrollBar::setNum(int i)
{
  num=i;
  value=0;
  setScrollNoob();
}

void UIScrollBar::setScrollNoob( )
{
  if(num)
  {
    ScrollKnob->y=  11.0f + ( (this->height - 54.0f) / (num-1) ) *value;
  }
}
