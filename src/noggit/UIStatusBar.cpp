#include <noggit/UIStatusBar.h>

#include <vector>
#include <string>

#include <noggit/blp_texture.h>
#include <noggit/application.h> // arial16
#include <noggit/UIText.h>

UIStatusBar::UIStatusBar( float xPos, float yPos, float w, float h )
: UIWindow( xPos, yPos, w, h )
, leftInfo( new UIText( 8.0f, 7.0f, "", arial16, eJustifyLeft ) )
, rightInfo( new UIText( width() - 8.0f, 7.0f, "", arial16, eJustifyRight ) )
{
  addChild( leftInfo );
  addChild( rightInfo );
}

void UIStatusBar::render() const
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

  UIFrame::renderChildren();

  glColor3f( 0.7f, 0.7f, 0.7f );

  opengl::texture::enable_texture (0);

  texture->bind();

  //Draw Top Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.375f, 1.0f );
  glVertex2f( 0.0f, 13.0f );
  glTexCoord2f( 0.375f, 0.0f );
  glVertex2f( width(), 13.0f );
  glTexCoord2f( 0.25f, 1.0f );
  glVertex2f( 0.0f, -3.0f );
  glTexCoord2f( 0.25f, 0.0f );
  glVertex2f( width(), -3.0f );
  glEnd();

  opengl::texture::disable_texture (0);

  glPopMatrix();
}

void UIStatusBar::setLeftInfo( const std::string& pText )
{
  leftInfo->setText( pText );
}

void UIStatusBar::setRightInfo( const std::string& pText )
{
  rightInfo->setText( pText );
}
