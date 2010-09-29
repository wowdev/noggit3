#ifndef __ICON_H
#define __ICON_H

#include "frame.h"

class Texture;

class Icon:public frame
{
protected:
	Texture* texture;
	Texture* textureSelected;
	void (*clickFunc)(frame *,int);
	int		id;


public:
	Icon(float x,float y,float width,float height, const std::string& tex, const std::string& texd);
	void	render() const;
	bool	selected;
	frame *processLeftClick(float mx,float my);
	void setClickFunc(void (*f)(frame *,int), int num);
};
#endif
