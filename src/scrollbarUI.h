#ifndef __SCROLLBARUI_H
#define __SCROLLBARUI_H

#include "buttonUI.h"
#include "textureUI.h"
#include "video.h"

class scrollbarUI:public frame
{
protected:
	buttonUI	*ScrollUp;
	buttonUI	*ScrollDown;
	textureUI	*ScrollKnob;
	int			num;
	int			value;
	void		(*changeFunc)(int);	
public:
	void	clickReturn(int);
	scrollbarUI(float xpos, float ypos, float height, int num);
	int		getValue(){return value;};
	void	setValue(int i)
	{
		value=i;
		if(value>=num)
			value=num-1;
		if(value<0)
			value=0;
		if(num>0)
			ScrollKnob->y=14.0f+(height-48.0f)*value/num;
	};
	void	setNum(int i)
	{
		num=i;
		if(value>=num)
			value=num-1;
	};
	void	setChangeFunc(void (*f)(int));
};
#endif