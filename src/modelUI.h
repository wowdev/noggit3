#ifndef MODELUI_H
#define MODELUI_H

#include "video.h"
#include "frame.h"
#include "model.h"

class modelUI:public frame
{
protected:
	void (*clickFunc)(frame *,int);
	int	id;
	char ch;
	Model *model;
public:
	modelUI(float x,float y,float width,float height);
	void	render();
	void	setModel(Model* setModel);

	frame *processLeftClick(float mx,float my);
	void	setClickFunc(void (*f)(frame *,int), int num);
};
#endif
