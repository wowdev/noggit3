#ifndef __WINDOW_H
#define __WINDOW_H

#include "video.h"
#include "frame.h"

class window : public frame
{
protected:
	GLuint	texture;

public:
	window( float xPos, float yPos, float w, float h );
	window( float xPos, float yPos, float w, float h, std::string pTexture );
	frame *processLeftClick( float mx, float my );
	void render( );	
};

#endif