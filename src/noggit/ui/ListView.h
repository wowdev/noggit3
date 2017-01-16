// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Frame.h>
#include <noggit/ui/ScrollBar.h>

class UIListView : public UIFrame
{
public:
	typedef UIListView* Ptr;

private:
	int elements_height;
	int elements_start;
	int elements_rows;
	UIScrollBar::Ptr scrollbar;

public:
	UIListView(float xPos, float yPos, float w, float h, int elementHeight);
	void addElement(UIFrame* element);
	void setElementsHeight(int h);
	int getElementsCount();
	void recalcElements(unsigned int value);
	void clear();
};
