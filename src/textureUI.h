#ifndef __TEXTUREUI_H
#define __TEXTUREUI_H

#include <string>

#include "video.h"
#include "frame.h"

class textureUI : public frame
{
protected:
	GLuint texture;
	bool highlight;
	void (*clickFunc)(frame *,int);
	int id;

public:
	textureUI( float x, float y, float width, float height, GLuint tex );
	textureUI( float x, float y, float width, float height, const std::string& tex );
	void setTexture( GLuint tex );
	void render( );

	frame *processLeftClick( float mx, float my );
	void setClickFunc( void (*f)( frame *,int ), int num );
    void setHighlight( bool h )
	{
		highlight = h;
	}
};

#endif
