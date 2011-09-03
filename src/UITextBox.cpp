#include "UITextBox.h"

#include <SDL.h>
#include <sstream>
#include <string>
#include <utf8.h>

#include "Noggit.h" // arial12
#include "TextureManager.h" // TextureManager, Texture
#include "UIText.h"
#include "UITexture.h"
#include "Video.h"

// TODO : Handle Selection, Handle Clipboard ( CTRL + C / CTRL + V / CTRL + X ), Handle the Backspace staying down. Details, but better like that.

UITextBox::UITextBox( float xPos, float yPos, float w, float h, const std::string& tex, const std::string& texd )
: UIFrame( xPos, yPos, w, h )
, texture( TextureManager::newTexture( tex ) )
, textureDown( TextureManager::newTexture( texd ) )
, _textureFilename( tex )
, _textureDownFilename( texd )
, mFocus( false )
, mText( new UIText( 8.0f, 2.5f, arial16, eJustifyLeft ) )
, mValue( "" )
{

}

UITextBox::~UITextBox()
{
  TextureManager::delbyname( _textureFilename );
  TextureManager::delbyname( _textureDownFilename );
}

void UITextBox::render() const
{
  glPushMatrix();
  glTranslatef( x(), y(), 0.0f );

  glColor3f( 1.0f, 1.0f, 1.0f );

  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();

  if( !mFocus )
    texture->bind();
  else
    textureDown->bind();

  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( 0.0f, 0.0f );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( width(), 0.0f );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( 0.0f, height() );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width(), height() );
  glEnd();

  OpenGL::Texture::disableTexture();
  mText->render();

  glPopMatrix();
}

UIFrame *UITextBox::processLeftClick( float /*mx*/, float /*my*/ )
{
  mFocus = !mFocus;
  return this;
}

void UITextBox::setValue( const std::string& pText )
{
  mValue = pText;
  mText->setText( mValue );
}

std::string UITextBox::getValue()
{
  return mValue;
}

bool UITextBox::KeyBoardEvent( SDL_KeyboardEvent *e )
{
  if( !mFocus )
    return false;

  if( e->type != SDL_KEYDOWN )
    return false;

  if( e->keysym.sym == SDLK_BACKSPACE && !mValue.empty()) // Backspace
  {
    const char* firstBeforeEnd( mValue.c_str() + mValue.length() );
    utf8::prior( firstBeforeEnd, mValue.c_str() );
    mValue.erase( firstBeforeEnd - mValue.c_str() );
  }
  else
  {
    if( e->keysym.sym == SDLK_RETURN ) // Enter
	  {
        mFocus = false;
	  }
    else
	  {
      if( e->keysym.unicode > 31 )
	    {
        utf8::append( e->keysym.unicode, std::back_inserter( mValue ) );
	    }
	  }
  }

  setValue( mValue );
  return true;
}

void UITextBox::Clear()
{
	setValue("");
}
