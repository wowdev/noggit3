#ifndef __BUTTONUI_H
#define __BUTTONUI_H

#include "frame.h"

namespace OpenGL { class Texture; };
class textUI;
namespace freetype { class font_data; };
	
class buttonUI : public frame
{
protected:
	OpenGL::Texture* texture;
	OpenGL::Texture* textureDown;
	void ( *clickFunc )( frame *, int );
	int	id;

	bool clicked;
	textUI *text;

public:
	buttonUI( float x, float y, float width, float height, const std::string& pTexNormal, const std::string& pTexDown );
	buttonUI( float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown );
	buttonUI( float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown, void (*pFunc)( frame *, int ), int pFuncParam );
	
	void render() const;

	void setLeft();
	void setText( const std::string& pText );
	void setFont( freetype::font_data *font );

	frame *processLeftClick( float mx, float my );
	void setClickFunc( void (*f)( frame *, int ), int num );
	void processUnclick();
};
#endif
