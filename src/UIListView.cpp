#include "UIListView.h"

#include <vector>

#include "Log.h"
#include "Misc.h"
#include "UIScrollBar.h"

void changeValue(UIFrame *f,int set)
{
  (reinterpret_cast<UIListView *>(f->parent))->recalcElements(set+1);
}

UIListView::UIListView(float xPos, float yPos, float w, float h, int elementHeight) : UIFrame(xPos,yPos,w,h)
{
  this->elements_height = elementHeight;
  this->scrollbar = new UIScrollBar(w-22,5,h-10,0);
  this->scrollbar->clickable = true;
  this->scrollbar->setChangeFunc(changeValue);
  this->addChild(scrollbar);
  this->elements_rows = h / elementHeight;
}

UIListView::~UIListView(void)
{
}

void UIListView::clear()
{
  // clear all elements except the first (scroll pan)
  for( std::vector<UIFrame*>::iterator child = children.begin(); child != children.end(); child++ )
  {
    if(this->children.size()!=1)
      this->children.erase( child );
  }
  this->scrollbar->setNum(0);
}

void UIListView::addElement( UIFrame *element )
{
  element->x = 4;
  element->y = 0;
  element->height = this->elements_height;
  element->width = this->width-20;
  this->addChild(element);
  this->scrollbar->setNum(this->children.size()-this->elements_rows);
  recalcElements(1);
}

int UIListView::getElementsCount()
{
  return this->children.size();
}

void UIListView::recalcElements(unsigned int value)
{
  this->elements_start = value;
  // recalculate the position and the hide value off all child.
  int rowCount = 0;
  for(unsigned int i=1;i<children.size();++i)
  {
    if(i >= value && i < value + this->elements_rows)
    {
      // elements in the view block
      children[i]->y = rowCount * this->elements_height;
      children[i]->hidden = false;
      rowCount++;
    }
    else children[i]->hidden = true;
  }
}