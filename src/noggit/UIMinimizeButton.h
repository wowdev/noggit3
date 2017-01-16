// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UIButton.h>

class UIMinimizeButton : public UIButton
{
public:
	typedef UIMinimizeButton* Ptr;

	UIMinimizeButton(float pWidth);

	UIFrame::Ptr processLeftClick(float mx, float my);
};
