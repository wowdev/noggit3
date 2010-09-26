#ifndef __SLIDER_H
#define __SLIDER_H

#include "frame.h"

class Texture;

class slider:public frame
{
protected:
	Texture* texture;
	Texture* sliderTexture;
	float	scale;
	float	offset;
	void (*func)(float);
	char	text[255];
public:
	float	value;
	void	setFunc(void (*f)(float));
	void	setValue(float f);
	void	setText(const char *);
	slider(float x, float y, float width, float s,float o);
	frame *processLeftClick(float mx,float my);
	bool processLeftDrag(float mx,float my, float xChange, float yChange);
	void render();	
};
#endif
