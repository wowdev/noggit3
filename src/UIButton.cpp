#include "UIButton.h"

#include <string>

#include "FreeType.h"
#include "Noggit.h" // arial12
#include "TextureManager.h" // TextureManager, Texture
#include "UIText.h"

UIButton::UIButton( float pX, float pY, float w, float h, const std::string& pTexNormal, const std::string& pTexDown )
{
  x = pX;
  y = pY;
  width = w;
  height = h;
  texture = TextureManager::newTexture( pTexNormal );
  textureDown = TextureManager::newTexture( pTexDown );
  clickFunc = NULL;
  id = 0;
  clicked = false;
  text = new UIText( w / 2.0f, 2.0f, arial12, eJustifyCenter );
  addChild( text );
}

UIButton::UIButton( float pX, float pY, float w, float h, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown )
{
  x = pX;
  y = pY;
  width = w;
  height = h;
  texture = TextureManager::newTexture( pTexNormal );
  textureDown = TextureManager::newTexture( pTexDown );
  clickFunc = NULL;
  id = 0;
  clicked = false;
  text = new UIText( w / 2.0f, 2.0f, pText, arial12, eJustifyCenter );
  addChild( text );
}

UIButton::UIButton( float pX, float pY, float w, float h, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown, void (*pFunc)( UIFrame *, int ), int pFuncParam )
{
  x = pX;
  y = pY;
  width = w;
  height = h;
  texture = TextureManager::newTexture( pTexNormal );
  textureDown = TextureManager::newTexture( pTexDown );
  clickFunc = pFunc;
  id = pFuncParam;
  clicked = false;
  text = new UIText( w / 2.0f, 2.0f, pText, arial12, eJustifyCenter );
  addChild( text );
}

void UIButton::setLeft()
{
  text->setJustify( eJustifyLeft );
  text->x = 10.0f;
}

void UIButton::setText( const std::string& pText )
{
  text->setText( pText );
}

void UIButton::setFont( freetype::font_data *font )
{
  text->setFont( font );
}
void UIButton::render() const
{
  glPushMatrix();
  glTranslatef( x, y, 0.0f );

  glColor3f( 1.0f, 1.0f, 1.0f );
  
  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();

  if( !clicked )
    texture->render();
  else
    textureDown->render();
  
  glBegin( GL_TRIANGLE_STRIP );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( 0.0f, 0.0f );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2f( width, 0.0f );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2f( 0.0f, height );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2f( width, height );
  glEnd();
  
  OpenGL::Texture::disableTexture();
  
  text->render();

  glPopMatrix();
}

UIFrame *UIButton::processLeftClick( float /*mx*/, float /*my*/ )
{
  clicked = true;
  if( clickFunc )
    clickFunc( this, id );
  return this;
}

void UIButton::processUnclick()
{
  clicked = false;
}

void UIButton::setClickFunc( void (*f)( UIFrame *, int ), int num )
{
  clickFunc = f;
  id = num;
}
