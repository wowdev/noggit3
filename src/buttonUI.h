#ifndef __BUTTONUI_H
#define __BUTTONUI_H

#include "video.h"
#include "frame.h"
#include "textUI.h"
#include "FreeType.h"

class buttonUI : public frame
{
protected:
	GLuint texture;
	GLuint textureDown;
	void ( *clickFunc )( frame *, int );
	int	id;

	bool clicked;
	textUI *text;

public:
	buttonUI( float x, float y, float width, float height, GLuint tex, GLuint texd );
	buttonUI( float x, float y, float width, float height, std::string pText, GLuint tex, GLuint texd );
	buttonUI( float x, float y, float width, float height, std::string pTexNormal, std::string pTexDown );
	buttonUI( float x, float y, float width, float height, std::string pText, std::string pTexNormal, std::string pTexDown );
	void render( );

	void setLeft( );
	void setText( std::string pText );
	void setFont( freetype::font_data *font );

	frame *processLeftClick( float mx, float my );
	void setClickFunc( void (*f)( frame *, int ), int num );
	void processUnclick( );
};
#endif
