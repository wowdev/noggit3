#include "ui_ListView.h"
#include "scrollbarUI.h"

ui_ListView::ui_ListView(float xPos, float yPos, float w, float h) : frame(xPos,yPos,w,h)
{
	scrollbarUI *scrollbar = new scrollbarUI(w-22,5,h-10,100);
	scrollbar->clickable = true;
	this->addChild(scrollbar);
}


ui_ListView::~ui_ListView(void)
{
}

void ui_ListView::addElement( frame element )
{
	element.height = this->elements_height;
	element.width = this->width-20;
	this->addChild(&element);
}

void ui_ListView::delElement( int num )
{

}

void ui_ListView::setElementsHeight( int h )
{
	this->elements_height = h;
}

int ui_ListView::getElementsCount()
{
	return this->children.size();
}
