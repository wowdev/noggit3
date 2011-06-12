#include "ToolbarIcon.h"

#include <string>

#include "video.h" // gl*
#include "TextureManager.h" // TextureManager, Texture

ToolbarIcon::ToolbarIcon( float xPos, float yPos, float w, float h, const std::string& tex, const std::string& texd, const int& id, UIEventClassConstructorArguments ) : frame( xPos, yPos, w, h ), UIEventClassConstructorSuperCall(), iconId( id ), selected( false )
{
  texture = TextureManager::newTexture( tex );
  textureSelected = TextureManager::newTexture( texd );
}

frame* ToolbarIcon::processLeftClick(float /*mx*/,float /*my*/)
{
  UIEventEventHandlerCall(iconId);
  
  return this;
}

void ToolbarIcon::render() const
{
  glColor3f(1.0f,1.0f,1.0f);
  
  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();
  
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

  OpenGL::Texture::disableTexture();
  
  if(selected)
  {
    static const int sizer = 18;
    
    OpenGL::Texture::enableTexture();
    
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
    
    OpenGL::Texture::disableTexture();
  }

  glPushMatrix();
  glTranslatef(x,y,0.0f);
  glPopMatrix();
}
