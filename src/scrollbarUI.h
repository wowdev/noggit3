#ifndef __SCROLLBARUI_H
#define __SCROLLBARUI_H

#include "frame.h"
#include "textUI.h"
#include "noggit.h" // arial14, arialn13

class textureUI;

class scrollbarUI:public frame
{
protected:
	int	 * mTarget;
	int		num;
	int		value;
	void ( *changeFunc )( frame * sender, int value );
	textureUI* ScrollKnob;
public:
	int		*extValue;
	void	clickReturn(int v);
	scrollbarUI(float xpos, float ypos, float height, int num);
	int		getValue() const;
	void	setValue(int i);
	void	setNum(int i);
	bool	processLeftDrag(float mx,float my, float xChange, float yChange);
	frame *processLeftClick(float mx,float my);
	void	setChangeFunc( void (*f)( frame * sender, int value));
	void	setScrollNoob( );
};
#endif
