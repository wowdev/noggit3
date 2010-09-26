#ifndef __WINDOW_H
#define __WINDOW_H

#include "frame.h"

class Texture;

class window : public frame
{
protected:
	Texture* texture;

public:
	window( float xPos, float yPos, float w, float h );
	//window( float xPos, float yPos, float w, float h, const std::string& pTexture );
	frame *processLeftClick( float mx, float my );
	void render();	
};

#endif
