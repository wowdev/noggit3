#ifndef __SCROLLBARUI_H
#define __SCROLLBARUI_H

#include "frame.h"

class textureUI;

class scrollbarUI:public frame
{
protected:
	int	 * mTarget;
	int	 num;
	int			value;
	void		(*changeFunc)(int);
	textureUI* ScrollKnob;
public:
	int *extValue;
	void	clickReturn(int);
	scrollbarUI(float xpos, float ypos, float height, int num);

	int		getValue() const;
	void	setValue(int i);
	void	setNum(int i);
	void	setChangeFunc(void (*f)(int));
	bool	processLeftDrag(float mx,float my, float xChange, float yChange);
	frame *processLeftClick(float mx,float my);
};
#endif
