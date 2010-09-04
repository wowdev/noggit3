#include "Icon.h"

Icon::Icon(float xPos,float yPos,float w, float h,GLuint tex, GLuint texd)
{
	x=xPos;
	y=yPos;
	width=w;
	height=h;
	texture=tex;
	textureSelected=texd;
	clickFunc=0;
	id=0;
	selected=false;
}

void Icon::render()
{
	glColor3f(1.0f,1.0f,1.0f);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, texture);
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
	if(this->selected==true)
	{
		int sizer = 18;
		glBindTexture(GL_TEXTURE_2D, textureSelected);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0f,0.0f);
		glVertex2f(x-sizer,y-sizer);
		glTexCoord2f(1.0f,0.0f);
		glVertex2f(x+width+sizer,y-sizer);
		glTexCoord2f(0.0f,1.0f);
		glVertex2f(x-sizer,y+height+sizer);
		glTexCoord2f(1.0f,1.0f);
		glVertex2f(x+width+sizer,y+height+sizer);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

	glPushMatrix();
	glTranslatef(x,y,0.0f);
	glPopMatrix();
}

frame *Icon::processLeftClick(float mx,float my)
{
	if(clickFunc)
		clickFunc(this,id);
	return this;
}

void Icon::setClickFunc(void (*f)(frame *,int), int num)
{
	clickFunc=f;
	id=num;
}
