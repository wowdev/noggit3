#include "UIScrollBar.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "Log.h"
#include "Misc.h"
#include "Noggit.h" // app.getArial14(), arialn13
#include "UIButton.h"
#include "UIText.h"
#include "UITexture.h"

const float UIScrollBar::WIDTH = 16.0f;

void scrollbarProcessClick(UIFrame::Ptr f, int id)
{
	(static_cast<UIScrollBar::Ptr>(f->parent()))->clickReturn(id);
}

UIScrollBar::UIScrollBar(float xpos, float ypos, float h, int n, Orientation orientation)
	: UIFrame(xpos, ypos, 0.0f, 0.0f)
	, mTarget()
	, num(n)
	, value(0)
	, changeFunc(NULL)
	, ScrollKnob(NULL)
	, _orientation(orientation)
	, extValue()
{
	UIButton::Ptr ScrollUp(NULL);
	UIButton::Ptr ScrollDown(NULL);

	if (_orientation == Vertical)
	{
		width(UIScrollBar::WIDTH);
		height(h);
		ScrollUp = new UIButton(-6.0f, -8.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Down.blp");
		ScrollDown = new UIButton(-6.0f, height() - 24.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Down.blp");
		ScrollKnob = new UITexture(-6.0f, 10.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-Knob.blp");
	}
	else
	{
		width(h);
		height(UIScrollBar::WIDTH);
		//! \todo filenames for buttons!
		ScrollUp = new UIButton(-8.0f, -6.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Down.blp");
		ScrollDown = new UIButton(width() - 24.0f, -6.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Down.blp");
		ScrollKnob = new UITexture(10.0f, -6.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-Knob.blp");
	}

	addChild(ScrollUp);
	addChild(ScrollDown);
	addChild(ScrollKnob);

	ScrollUp->setClickFunc(scrollbarProcessClick, -1);
	ScrollKnob->setClickFunc(scrollbarProcessClick, 0);
	ScrollDown->setClickFunc(scrollbarProcessClick, 1);
}

bool UIScrollBar::processLeftDrag(float mx, float my, float /*xChange*/, float /*yChange*/)
{
	if (num < 0)
		return false;

	float tx(0.0f);
	float ty(0.0f);
	getOffset(&tx, &ty);
	mx -= tx + 32.0f;
	my -= ty + 32.0f;

	if (_orientation == Vertical)
		value = std::min(num - 1, std::max(0, misc::FtoIround(num * my / (height() - 64.0f))));
	else
		value = std::min(num - 1, std::max(0, misc::FtoIround(num * mx / (width() - 64.0f))));

	setScrollNoob();

	if (changeFunc)
	{
		changeFunc(this, value);
	}

	return true;
}

UIFrame::Ptr UIScrollBar::processLeftClick(float mx, float my)
{
	UIFrame::Ptr temp = UIFrame::processLeftClick(mx, my);
	if (temp == ScrollKnob)
		//! \note We want the drag event for the knob.
	{
		return this;
	}
	return temp;
}

void UIScrollBar::clickReturn(int id)
{
	if (id != 0)
	{
		value = std::min(num - 1, std::max(0, value + id));

		setScrollNoob();

		if (changeFunc)
		{
			changeFunc(this, value);
		}
	}
}

void UIScrollBar::setChangeFunc(void(*f)(UIFrame *, int))
{
	changeFunc = f;
}

int UIScrollBar::getValue() const
{
	return value;
}

void UIScrollBar::setValue(int i)
{
	if (i > -1 && i < num)
		value = i;
	setScrollNoob();
}

void UIScrollBar::setNum(int i)
{
	num = i;
	value = 0;
	setScrollNoob();
}

void UIScrollBar::setScrollNoob()
{
	if (num)
	{
		if (_orientation == Vertical)
			ScrollKnob->y(11.0f + ((height() - 54.0f) / (num - 1)) * value);
		else
			ScrollKnob->x(11.0f + ((width() - 54.0f) / (num - 1)) * value);
	}
}
