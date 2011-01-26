#include "Icon.h"
#include "video.h" // gl*
#include "TextureManager.h" // TextureManager, Texture

Icon::Icon(float xPos,float yPos,float w, float h, const std::string& tex,	const std::string& texd)
{
	x=xPos;
	y=yPos;
	width=w;
	height=h;
	texture = TextureManager::newTexture( tex );
	textureSelected = TextureManager::newTexture( texd );
	clickFunc=0;
	id=0;
	selected=false;
}

void Icon::render() const
{
	glColor3f(1.0f,1.0f,1.0f);
	
	Texture::setActiveTexture();
	Texture::enableTexture();
	
	texture->render();

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
	
	if(this->selected)
	{
		static const int sizer = 18;
		
		Texture::enableTexture();
		
		textureSelected->render();

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
		
		Texture::disableTexture();
	}

	glPushMatrix();
	glTranslatef(x,y,0.0f);
	glPopMatrix();
}

frame *Icon::processLeftClick(float /*mx*/,float /*my*/)
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
