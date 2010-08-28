#include "slider.h"
#include "noggit.h"

slider::slider(float xPos, float yPos, float w,float s,float o)
{
	movable=false;
	hidden=false;
	clickable=true;
	x=xPos;
	y=yPos;
	width=w;
	height=10.0f;
	value=0.5f;
	func=0;
	scale=s;
	offset=o;
	text[0]=0;
	texture=video.textures.add("Interface\\Buttons\\UI-SliderBar-Border.blp");
	sliderTexture=video.textures.add("Interface\\Buttons\\UI-SliderBar-Button-Horizontal.blp");
}

void slider::setFunc(void(*f)(float))
{
	func=f;
}

void slider::setValue(float f)
{
	if(f>1.0f)
		f=1.0f;
	else if(f<0.0f)
		f=0.0f;

	value=f;
	func(value*scale+offset);
}

void slider::setText(const char *t)
{
	strcpy(text,t);
}

frame *slider::processLeftClick(float mx,float my)
{
	/*if((mx>(width*value-16))&&(mx<(width*value+16)))
        return this;
	return 0;*/

	value=mx/width;
	if(value<0.0f)
		value=0.0f;
	else if(value>1.0f)
		value=1.0f;
	if(func)
		func(value*scale+offset);
	return this;
}

bool slider::processLeftDrag(float mx,float my, float xChange, float yChange)
{
	float tx,ty;

	parent->getOffset(tx,ty);
	mx-=tx;
	my-=ty;
	value=mx/width;
	if(value<0.0f)
		value=0.0f;
	else if(value>1.0f)
		value=1.0f;
	if(func)
		func(value*scale+offset);
	return true;
}


//8 parts to the border image from left to right
//left side
//right side
//top side
//bottom side
//Bottom Left corner
//Bottom Right Corner
//Top Left Corner
//Top Right Corner

void slider::render()
{
	if(hidden)
		return;
	glPushMatrix();
	glTranslatef(x,y,0);




	glColor3f(1.0f,1.0f,1.0f);
	
	

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	char Temp[255];

	if(text[0]!=0)
	{
		int twidth;
		//OGLFT::BBox	Box;
		sprintf(Temp,text,value*scale+offset);
	
		twidth=freetype::width(arial12,Temp);
		
		
		freetype::shprint(arial12,width/2-twidth/2,-16,Temp);	
	}
	
	glPushMatrix();

	glBindTexture(GL_TEXTURE_2D, texture);
	

	//Draw Bottom left Corner First
	glBegin(GL_TRIANGLE_STRIP);	
	glTexCoord2f(0.75f,1.0f);
	glVertex2f(-1,height+4);	
	glTexCoord2f(7.0f/8.0f,1.0f);
	glVertex2f(7,height+4);
	glTexCoord2f(0.75f,0.0f);
	glVertex2f(-1,height-4);
	glTexCoord2f(7.0f/8.0f,0.0f);
	glVertex2f(7,height-4);
	glEnd();

	//Draw Bottom Right Corner
	glBegin(GL_TRIANGLE_STRIP);	
	glTexCoord2f(7.0f/8.0f,1.0f);
	glVertex2f(width-7,height+4);	
	glTexCoord2f(8.0f/8.0f,1.0f);
	glVertex2f(width+1,height+4);
	glTexCoord2f(7.0f/8.0f,0.0f);
	glVertex2f(width-7,height-4);
	glTexCoord2f(8.0f/8.0f,0.0f);
	glVertex2f(width+1,height-4);
	glEnd();

	//Draw Top Left Corner
	glBegin(GL_TRIANGLE_STRIP);	
	glTexCoord2f(0.5f,1.0f);
	glVertex2f(-1,4);	
	glTexCoord2f(0.625f,1.0f);
	glVertex2f(7,4);
	glTexCoord2f(0.5f,0.0f);
	glVertex2f(-1,-4);
	glTexCoord2f(0.625f,0.0f);
	glVertex2f(7,-4);
	glEnd();

	//Draw Top Right Corner
	glBegin(GL_TRIANGLE_STRIP);	
	glTexCoord2f(0.625f,1.0f);
	glVertex2f(width-7,4);	
	glTexCoord2f(0.75f,1.0f);
	glVertex2f(width+1,4);
	glTexCoord2f(0.625f,0.0f);
	glVertex2f(width-7,-4);
	glTexCoord2f(0.75f,0.0f);
	glVertex2f(width+1,-4);
	glEnd();

	if(height>8)
	{
		//Draw Left Side
		glBegin(GL_TRIANGLE_STRIP);	
		glTexCoord2f(0.0f,1.0f);
		glVertex2f(-1,height-4);	
		glTexCoord2f(0.125f,1.0f);
		glVertex2f(7,height-4);
		glTexCoord2f(0.0f,0.0f);
		glVertex2f(-1,4);
		glTexCoord2f(0.125f,0.0f);
		glVertex2f(7,4);
		glEnd();

		//Draw Right Side
		glBegin(GL_TRIANGLE_STRIP);	
		glTexCoord2f(0.125f,1.0f);
		glVertex2f(width-7,height-4);	
		glTexCoord2f(0.25f,1.0f);
		glVertex2f(width+1,height-4);
		glTexCoord2f(0.125f,0.0f);
		glVertex2f(width-7,4);
		glTexCoord2f(0.25f,0.0f);
		glVertex2f(width+1,4);
		glEnd();
	}

	if(width>14)
	{
		//Draw Top Side
		glBegin(GL_TRIANGLE_STRIP);	
		glTexCoord2f(0.5f,1.0f);
		glVertex2f(7,height+4);	
		glTexCoord2f(0.5f,0.0f);
		glVertex2f(width-7,height+4);	
		glTexCoord2f(0.375f,1.0f);
		glVertex2f(7,height-4);
		glTexCoord2f(0.375f,0.0f);
		glVertex2f(width-7,height-4);
		glEnd();

		//Draw Bottom Side
		glBegin(GL_TRIANGLE_STRIP);	
		glTexCoord2f(0.375f,1.0f);
		glVertex2f(7,4);	
		glTexCoord2f(0.375f,0.0f);
		glVertex2f(width-7,4);
		glTexCoord2f(0.25f,1.0f);
		glVertex2f(7,-4);
		glTexCoord2f(0.25f,0.0f);
		glVertex2f(width-7,-4);
		glEnd();
	}

	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, sliderTexture);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0f,0.0f);
	glVertex2f(width*value-16.0f,height/2-16.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex2f(width*value+16.0f,height/2-16.0f);
	glTexCoord2f(0.0f,1.0f);
	glVertex2f(width*value-16.0f,height/2+16.0f);
	glTexCoord2f(1.0f,1.0f);
	glVertex2f(width*value+16.0f,height/2+16.0f);
	glEnd();
	

	glDisable(GL_TEXTURE_2D);


	glPopMatrix();
}