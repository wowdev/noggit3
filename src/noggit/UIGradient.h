// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "math/quaternion.hpp" // math::vector_4d
#include "UIFrame.h" // UIFrame

class UIGradient : public UIFrame
{
protected:
	math::vector_4d  MinColor;
	math::vector_4d  MaxColor;
	math::vector_4d  ClickColor;
	float  value;
	void(*clickFunc)(float val);

public:
	bool  horiz;

	void  setValue(float f);
	void  setClickFunc(void(*f)(float val));
	void  setMinColor(float r, float g, float b, float a);
	void  setMaxColor(float r, float g, float b, float a);
	void  setClickColor(float r, float g, float b, float a);
	UIFrame  *processLeftClick(float mx, float my);
	bool  processLeftDrag(float mx, float my, float xChange, float yChange);
	void  render() const;
};
