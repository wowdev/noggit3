#ifndef MODELUI_H
#define MODELUI_H

#include "frame.h"

class Model;

class modelUI:public frame
{
protected:
	Model *model;
public:
	modelUI(float x,float y,float width,float height);
	void	render() const;
	void	setModel(Model* setModel);
};
#endif
