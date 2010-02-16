#ifndef __ICON_H
#define __ICON_H

#include "video.h"
#include "frame.h"

class Icon:public frame
{
protected:
	GLuint	texture;
	GLuint	textureSelected;
	void (*clickFunc)(frame *,int);
	int		id;


public:
	Icon(float x,float y,float width,float height,GLuint tex,GLuint texd);
	void	render();
	bool	selected;
	frame *processLeftClick(float mx,float my);
	void setClickFunc(void (*f)(frame *,int), int num);
};
#endif