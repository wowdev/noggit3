#include "UIToolbarIcon.h"

#include <string>

#include "TextureManager.h" // TextureManager, Texture
#include "Video.h" // gl*

UIToolbarIcon::UIToolbarIcon( float xPos, float yPos, float w, float h, const std::string& tex, const std::string& texd, const int& id, UIEventClassConstructorArguments )
: UIFrame( xPos, yPos, w, h )
, UIEventClassConstructorSuperCall()
, texture( TextureManager::newTexture( tex ) )
, textureSelected( TextureManager::newTexture( texd ) )
, _textureFilename( tex )
, _textureSelectedFilename( texd )
, iconId( id )
, selected( false )
{
}

UIToolbarIcon::~UIToolbarIcon()
{
  TextureManager::delbyname( _textureFilename );
  TextureManager::delbyname( _textureSelectedFilename );
}

UIFrame* UIToolbarIcon::processLeftClick( float /*mx*/, float /*my*/ )
{
  UIEventEventHandlerCall(iconId);
  
  return this;
}

void UIToolbarIcon::render() const
{
  glColor3f(1.0f,1.0f,1.0f);
  
  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();
  
  texture->bind();

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
    static const float sizer = 18.0f;
    
    OpenGL::Texture::enableTexture();
    
    textureSelected->bind();

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
