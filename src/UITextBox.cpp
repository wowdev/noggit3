#include "UITextBox.h"

#include <SDL.h>
#include <sstream>
#include <string>

#include "Noggit.h" // arial12
#include "TextureManager.h" // TextureManager, Texture
#include "UIText.h"
#include "UITexture.h"
#include "Video.h"

UITextBox2::UITextBox2( float xPos, float yPos, float w, float h, const std::string& tex, const std::string& texd )
: UIFrame( xPos, yPos, w, h )
, texture( TextureManager::newTexture( tex ) )
, textureDown( TextureManager::newTexture( texd ) )
, _textureFilename( tex )
, _textureDownFilename( texd )
, mFocus( false )
, mText( new UIText( w / 2.0f, 2.0f, arial12, eJustifyCenter ) )
, mValue( "" )
{
}

UITextBox2::~UITextBox2()
{
  TextureManager::delbyname( _textureFilename );
  TextureManager::delbyname( _textureDownFilename );
}

void UITextBox2::render() const
{
  glColor3f(1.0f,1.0f,1.0f);
  
  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();
  
  if( !mFocus )
    texture->bind();
  else
    textureDown->bind();

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
  
  glPushMatrix();
  glTranslatef(x,y,0.0f);
  mText->render();
  glPopMatrix();
}

UIFrame *UITextBox2::processLeftClick( float /*mx*/, float /*my*/ )
{
  mFocus = !mFocus;
  return this;
}

UITextBox::UITextBox(float xpos, float ypos, float w)
{
  x=xpos;
  y=ypos;
  width=w;
  height=32;
  background=new UITexture(0,0,w,32,"Interface\\Common\\Common-Input-Border.blp");
}
UIFrame *UITextBox::processLeftClick(float /*mx*/,float /*my*/)
{
  return this;
}
bool UITextBox::processKey(char key, bool /*shift*/, bool /*alt*/, bool /*ctrl*/)
{
  text[length]=key;text[length+1]=0;length++;return true;
}

void UITextBox2::setValue( const std::string& pText )
{
  mValue = pText;
  mText->setText( mValue );
}
std::string UITextBox2::getValue()
{
  return mValue;
}
  
bool UITextBox2::KeyBoardEvent( SDL_KeyboardEvent *e )
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
        mValue += static_cast<char>( e->keysym.sym );
      else
      {
        std::stringstream ss; ss << "\\x" << e->keysym.sym;
        mValue += ss.str();
      }
  
  setValue( mValue );
  return true;
}
