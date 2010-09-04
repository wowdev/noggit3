#ifndef __TEXTBOXUI_H
#define __TEXTBOXUI_H

#include "video.h"
#include "frame.h"
#include "textureUI.h"
#include "textUI.h"
#include <sstream>

//! \todo  Combine and get it working.
class textboxUI:public frame
{
protected:
	char		text[256];
	int			length;
	textureUI	*background;
	textUI		*theText;
public:
	textboxUI(float xpos, float ypos, float w)
	{
		x=xpos;
		y=ypos;
		width=w;
		height=32;
		background=new textureUI(0,0,w,32,video.textures.add("Interface\\Common\\Common-Input-Border.blp"));
	};
	frame *processLeftClick(float mx,float my){return this;};
	bool processKey(char key, bool shift, bool alt, bool ctrl){text[length]=key;text[length+1]=0;length++;return true;};
	void render();
};

class TextBox : public frame
{
private:
	GLuint	texture;
	GLuint	textureDown;

	bool	mFocus;
	textUI	*mText;
	std::string mValue;
public:
	TextBox(float xPos,float yPos,float w, float h,GLuint tex, GLuint texd);
	void render();

	void setValue( std::string pText )
	{
		mValue = pText;
		mText->setText( mValue.c_str() );
	}
	std::string	getValue( )
	{
		return mValue;
	}

	bool KeyBoardEvent( SDL_KeyboardEvent *e )
	{
		// The input is fixed to be ascii. its not really "working" with other keyboard layouts. maybe do this somehow else .. but how? ._. stupid SDL.

		if( !mFocus )
			return false;
		if( e->type != SDL_KEYDOWN )
			return false;

		if( e->keysym.sym == 8 )
			mValue = mValue.substr( 0, mValue.size() - 1 );
		else
			if( e->keysym.sym == 13 )
				mFocus = false;
			else
				if( e->keysym.sym < 127 && e->keysym.sym > 31 )
					mValue += char( e->keysym.sym );
				else
				{
					std::stringstream ss; ss << "\\x" << e->keysym.sym;
					mValue += ss.str();
				}

		setValue( mValue );
		return true;
	}
	
	frame *processLeftClick( float mx, float my );
	void processUnclick() { }
};
#endif
