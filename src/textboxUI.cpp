#include <SDL/SDL.h>
#include <sstream>

#include "textboxUI.h"
#include "noggit.h" // arial12

#include "video.h"
#include "textureUI.h"
#include "textUI.h"
#include "TextureManager.h" // TextureManager, Texture

TextBox::TextBox(float xPos,float yPos,float w, float h, const std::string& tex, const std::string& texd)
{
	x=xPos;
	y=yPos;
	width=w;
	height=h;
	texture = TextureManager::newTexture( tex );
	textureDown = TextureManager::newTexture( texd );
	mFocus = false;
	mText = new textUI( w / 2.0f, 2.0f, &arial12, eJustifyCenter );
}

void TextBox::render() const
{
	glColor3f(1.0f,1.0f,1.0f);
	
	Texture::setActiveTexture();
	Texture::enableTexture();
	
	if( !mFocus )
		texture->render();
	else
		textureDown->render();

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0f,0.0f);
	glVertex2f(x,y);
	glTexCoord2f(1.0f,0.0f);
	glVertex2f(x+width,y);
	glTexCoord2f(0.0f,1.0f);
	glVertex2f(x,y+height);
	glTexCoord2f(1.0f,1.0f);
	glVertex2f(x+width,y+height);
	glEnd();
	
	Texture::disableTexture();
	
	glPushMatrix();
	glTranslatef(x,y,0.0f);
	mText->render();
	glPopMatrix();
}

frame *TextBox::processLeftClick( float /*mx*/, float /*my*/ )
{
	mFocus = !mFocus;
	return this;
}

textboxUI::textboxUI(float xpos, float ypos, float w)
{
	x=xpos;
	y=ypos;
	width=w;
	height=32;
	background=new textureUI(0,0,w,32,"Interface\\Common\\Common-Input-Border.blp");
}
frame *textboxUI::processLeftClick(float /*mx*/,float /*my*/)
{
	return this;
}
bool textboxUI::processKey(char key, bool /*shift*/, bool /*alt*/, bool /*ctrl*/)
{
	text[length]=key;text[length+1]=0;length++;return true;
}

void TextBox::setValue( const std::string& pText )
{
	mValue = pText;
	mText->setText( mValue );
}
std::string	TextBox::getValue()
{
	return mValue;
}
	
bool TextBox::KeyBoardEvent( SDL_KeyboardEvent *e )
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
