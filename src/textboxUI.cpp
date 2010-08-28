#include "textboxUI.h"
#include "noggit.h"

TextBox::TextBox(float xPos,float yPos,float w, float h,GLuint tex, GLuint texd)
{
	x=xPos;
	y=yPos;
	width=w;
	height=h;
	texture=tex;
	textureDown=texd;
	mFocus = false;
	mText = new textUI( w / 2.0f, 2.0f, &arial12, eJustifyCenter );
}

void TextBox::render()
{
	glColor3f(1.0f,1.0f,1.0f);

	glActiveTexture(GL_TEXTURE0);
	if(!mFocus)
		glBindTexture(GL_TEXTURE_2D, texture);
	else
		glBindTexture(GL_TEXTURE_2D, textureDown);
	glEnable(GL_TEXTURE_2D);
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

	glDisable(GL_TEXTURE_2D);
	
	glPushMatrix();
	glTranslatef(x,y,0.0f);
	mText->render();
	glPopMatrix();
}

frame *TextBox::processLeftClick( float mx, float my )
{
	mFocus = !mFocus;
	return this;
}
