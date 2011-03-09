#ifndef __UILISTVIEW_H
#define __UILISTVIEW_H

#include "frame.h"
#include "scrollbarUI.h"

class ui_ListView : public frame
{
public:
	ui_ListView(float xPos, float yPos, float w, float h, int elementHeight);
	~ui_ListView(void);
	void addElement(frame *element);
	void delElement(int num);
	void setElementsHeight(int h);
	int	 getElementsCount();
	void recalcElements(unsigned int value);
private:
	int elements_height;
	int elements_start;
	int elements_rows;
	scrollbarUI *scrollbar;

};

#endif