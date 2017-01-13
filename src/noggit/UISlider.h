// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>
#include <boost/function.hpp>

#include "UIFrame.h"

namespace OpenGL { class Texture; }

class UISlider : public UIFrame
{
protected:
	OpenGL::Texture* texture;
	OpenGL::Texture* sliderTexture;
	float scale;
	float offset;
	boost::function<void(float)> func;
	std::string text;

public:
	float value;
	void setFunc(void(*f)(float value));
	void setFunc(boost::function<void(float value)> pFunc);
	void setValue(float f);
	void setText(const std::string& text);
	UISlider(float x, float y, float width, float s, float o);
	~UISlider();
	UIFrame* processLeftClick(float mx, float my);
	bool processLeftDrag(float mx, float my, float xChange, float yChange);
	void render() const;
};
