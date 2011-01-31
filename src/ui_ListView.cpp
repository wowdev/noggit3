#include "ui_ListView.h"
#include "scrollbarUI.h"
#include "log.h"
#include "misc.h"

void changeValue(frame *f,int set)
{
	((ui_ListView *)(f->parent))->recalcElements(set+1);
}

ui_ListView::ui_ListView(float xPos, float yPos, float w, float h, int elementHeight) : frame(xPos,yPos,w,h)
{
	this->elements_height = elementHeight;
	this->scrollbar = new scrollbarUI(w-22,5,h-10,0);
	this->scrollbar->clickable = true;
	this->scrollbar->setChangeFunc(changeValue);
	this->addChild(scrollbar);
	this->elements_rows = (int)( h / elementHeight);
}

ui_ListView::~ui_ListView(void)
{
}

void ui_ListView::addElement( frame *element )
{
	element->height = this->elements_height;
	element->width = this->width-20;
	this->addChild(element);
	this->scrollbar->setNum(this->children.size()-1);
	recalcElements(1);
}

int ui_ListView::getElementsCount()
{
	return this->children.size();
}

void ui_ListView::delElement( int num )
{
	//recalcElements();
}

void ui_ListView::recalcElements(int value)
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