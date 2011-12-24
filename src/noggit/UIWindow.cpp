#include <noggit/UIWindow.h>

#include <vector>
#include <string>

#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/Video.h>

UIWindow::UIWindow( float xPos, float yPos, float w, float h )
: UIFrame( xPos, yPos, w, h )
, texture( TextureManager::newTexture( "interface\\tooltips\\ui-tooltip-border.blp" ) )
, _textureFilename( "interface\\tooltips\\ui-tooltip-border.blp" )
{
}

UIWindow::UIWindow( float xPos, float yPos, float w, float h, const std::string& pTexture )
: UIFrame( xPos, yPos, w, h )
, texture( TextureManager::newTexture( pTexture ) )
, _textureFilename( pTexture )
{
}

UIWindow::~UIWindow()
{
  TextureManager::delbyname( _textureFilename );
}

UIFrame::Ptr UIWindow::processLeftClick( float mx, float my )
{
  UIFrame::Ptr lTemp( UIFrame::processLeftClick( mx, my ) );
  if( lTemp )
  {
    return lTemp;
  }
  return this;
}

void UIWindow::render() const
{
  glPushMatrix();
  glTranslatef( x(), y(), 0.0f );

  glColor4f( 0.2f, 0.2f, 0.2f, 0.8f );
  glBegin( GL_TRIANGLE_STRIP );
  glVertex2f( 0.0f, 0.0f );
  glVertex2f( width(), 0.0f );
  glVertex2f( 0.0f, height() );
  glVertex2f( width(), height() );
  glEnd();

  renderChildren();

  glColor3f( 1.0f, 1.0f, 1.0f );

  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();

  texture->bind();

  //Draw Bottom left Corner First
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.75f, 1.0f );
  glVertex2f( -3.0f, height() + 3.0f );
  glTexCoord2f( 0.875f, 1.0f );
  glVertex2f( 13.0f, height() + 3.0f );
  glTexCoord2f( 0.75f, 0.0f );
  glVertex2f( -3.0f, height() - 13.0f );
  glTexCoord2f( 0.875f, 0.0f );
  glVertex2f( 13.0f, height() - 13.0f );
  glEnd();

  //Draw Bottom Right Corner
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.875f, 1.0f );
  glVertex2f( width() - 13.0f, height() + 3.0f );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width() + 3.0f, height() + 3.0f );
  glTexCoord2f( 0.875f, 0.0f );
  glVertex2f( width() - 13.0f, height() - 13.0f );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( width() + 3.0f, height() - 13.0f );
  glEnd();

  //Draw Top Left Corner

  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.5f, 1.0f );
  glVertex2f( -3.0f, 13.0f );
  glTexCoord2f( 0.625f, 1.0f );
  glVertex2f( 13.0f, 13.0f );
  glTexCoord2f( 0.5f, 0.0f );
  glVertex2f( -3.0f, -3.0f );
  glTexCoord2f( 0.625f, 0.0f );
  glVertex2f( 13.0f, -3.0f );
  glEnd();

  //Draw Top Right Corner
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.625f, 1.0f );
  glVertex2f( width() - 13.0f, 13.0f );
  glTexCoord2f( 0.75f, 1.0f );
  glVertex2f( width() + 3.0f, 13.0f );
  glTexCoord2f( 0.625f, 0.0f );
  glVertex2f( width() - 13.0f, -3.0f );
  glTexCoord2f( 0.75f, 0.0f );
  glVertex2f( width() + 3.0f, -3.0f );
  glEnd();

  //Draw Left Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( -3.0f, height() - 13.0f );
  glTexCoord2f( 0.125f, 1.0f );
  glVertex2f( 13.0f, height() - 13.0f );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( -3.0f, 13.0f );
  glTexCoord2f( 0.125f, 0.0f );
  glVertex2f( 13, 13.0f );
  glEnd();

  //Draw Right Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.125f, 1.0f );
  glVertex2f( width() - 13.0f, height() - 13.0f );
  glTexCoord2f( 0.25f, 1.0f );
  glVertex2f( width() + 3.0f, height() - 13.0f );
  glTexCoord2f( 0.125f, 0.0f );
  glVertex2f( width() - 13.0f, 13.0f );
  glTexCoord2f( 0.25f, 0.0f );
  glVertex2f( width() + 3.0f, 13.0f );
  glEnd();

  //Draw Top Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.5f, 1.0f );
  glVertex2f( 13.0f, height() + 3.0f );
  glTexCoord2f( 0.5f, 0.0f );
  glVertex2f( width() - 13.0f, height() + 3.0f );
  glTexCoord2f( 0.375f, 1.0f );
  glVertex2f( 13, height() - 13.0f );
  glTexCoord2f( 0.375f, 0.0f );
  glVertex2f( width() - 13.0f, height() - 13.0f );
  glEnd();

  //Draw Bottom Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.375f, 1.0f );
  glVertex2f( 13.0f, 13.0f );
  glTexCoord2f( 0.375f, 0.0f );
  glVertex2f( width() - 13.0f, 13.0f );
  glTexCoord2f( 0.25f, 1.0f );
  glVertex2f( 13.0f, -3.0f );
  glTexCoord2f( 0.25f, 0.0f );
  glVertex2f( width() - 13.0f, -3.0f );
  glEnd();

  OpenGL::Texture::disableTexture();

  glPopMatrix();
}
