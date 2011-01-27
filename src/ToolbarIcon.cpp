#include "ToolbarIcon.h"
#include "video.h" // gl*
#include "TextureManager.h" // TextureManager, Texture
#include "log.h"

ToolbarIcon::ToolbarIcon( float xPos, float yPos, float w, float h, const std::string& tex, const std::string& texd, const int& id, EventHandlerType _eventHandler, UIEventListener* _listener ) : frame( xPos, yPos, w, h ), UIEventSender(reinterpret_cast<UIEventSender::EventHandlerType>(_eventHandler), _listener), iconId( id ), selected( false )
{
	char tmp3[100]; sprintf(tmp3,"n: %p\n", listener);
	LogDebug << "Konstruktor-Listener:" << tmp3 << std::endl;
	texture = TextureManager::newTexture( tex );
	textureSelected = TextureManager::newTexture( texd );
}

frame *ToolbarIcon::processLeftClick(float /*mx*/,float /*my*/)
{

  EventHandlerType eventHandlerInt = reinterpret_cast<ToolbarIcon::EventHandlerType>(eventHandler);
  char tmp1[100]; sprintf(tmp1,"n: %p\n", eventHandler);
  char tmp2[100]; sprintf(tmp2,"n: %p\n", eventHandlerInt);

	char tmp3[100]; sprintf(tmp3,"n: %p\n", listener);
	LogDebug << "evnt: " <<  tmp1 << ", casted: " << tmp2 << "processLeftClick-Listener:" << tmp3 << std::endl;
  (listener->*eventHandlerInt)(iconId);

  return this;
}

void ToolbarIcon::render() const
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
	
	if(selected)
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
