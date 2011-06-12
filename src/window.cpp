#include "window.h"

#include <vector>
#include <string>

#include "video.h"
#include "TextureManager.h" // TextureManager, Texture

window::window( float xPos, float yPos, float w, float h ) : frame( xPos, yPos, w, h ), texture( TextureManager::newTexture( "interface\\tooltips\\ui-tooltip-border.blp" ) )
{
}

window::window( float xPos, float yPos, float w, float h, const std::string& pTexture ) : frame( xPos, yPos, w, h ), texture( TextureManager::newTexture( pTexture ) )
{
  //! \todo save pTexture to release it in the desctructor.
}

frame* window::processLeftClick( float mx, float my )
{
  frame* lTemp;
  for( std::vector<frame*>::reverse_iterator child = children.rbegin(); child != children.rend(); child++ )
  {
    if( !( *child )->hidden && ( *child )->IsHit( mx, my ) )
    {
      lTemp = ( *child )->processLeftClick( mx - ( *child )->x, my - ( *child )->y );
      if( lTemp )
        return lTemp;
    }
  }
  return this;
}

void window::render() const
{
  //! \todo  Get this to work. Its supposed to cut elements outside of width and height.
  /*
  glClear( GL_STENCIL_BUFFER_BIT );
  glColorMask( false, false, false, false );
  glEnable(GL_STENCIL_TEST);

    glStencilFunc( GL_ALWAYS, 1, 1 );
    glStencilOp( GL_KEEP, GL_ZERO, GL_REPLACE );

    glColor4f(1.0f,0.2f,0.2f,0.8f);
    glBegin(GL_TRIANGLE_STRIP);
      glVertex2f(0,0);
      glVertex2f(width,0);
      glVertex2f(0,height);
      glVertex2f(width,height);
    glEnd();

    glColorMask( true, true, true, true );

    glStencilFunc( GL_NOTEQUAL, 1, 1 );
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    
    glColor4f(1.0f,0.2f,0.2f,0.8f);
    glBegin(GL_TRIANGLE_STRIP);
      glVertex2f(0,0);
      glVertex2f(width,0);
      glVertex2f(0,height);
      glVertex2f(width,height);
    glEnd();

    glColor4f(0.0f,1.0f,0.0f,1.0f);

    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0f,0.0f);
      glVertex2f(width-100,height-100);
      glTexCoord2f(1.0f,0.0f);
      glVertex2f(width+100,height-100);
      glTexCoord2f(0.0f,1.0f);
      glVertex2f(width-100,height+100);
      glTexCoord2f(1.0f,1.0f);
      glVertex2f(width+100,height+100);
    glEnd();


  glDisable( GL_STENCIL_TEST );*/

  glPushMatrix();
  glTranslatef( x, y, 0.0f );

  glColor4f( 0.2f, 0.2f, 0.2f, 0.8f );
  glBegin( GL_TRIANGLE_STRIP );
  glVertex2f( 0.0f, 0.0f );
  glVertex2f( width, 0.0f );
  glVertex2f( 0.0f, height );
  glVertex2f( width, height );
  glEnd();

  for( std::vector<frame*>::const_iterator child = children.begin(); child != children.end(); ++child )
    if( !( *child )->hidden )
      ( *child )->render();

  glColor3f( 1.0f, 1.0f, 1.0f );
  
  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();
  
  texture->render();

  //Draw Bottom left Corner First
  glBegin( GL_TRIANGLE_STRIP );  
  glTexCoord2f( 0.75f, 1.0f );
  glVertex2f( -3.0f, height + 3.0f );  
  glTexCoord2f( 0.875f, 1.0f );
  glVertex2f( 13.0f, height + 3.0f );
  glTexCoord2f( 0.75f, 0.0f );
  glVertex2f( -3.0f, height - 13.0f );
  glTexCoord2f( 0.875f, 0.0f );
  glVertex2f( 13.0f, height - 13.0f );
  glEnd();

  //Draw Bottom Right Corner
  glBegin( GL_TRIANGLE_STRIP );  
  glTexCoord2f( 0.875f, 1.0f );
  glVertex2f( width - 13.0f, height + 3.0f );  
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width + 3.0f, height + 3.0f );
  glTexCoord2f( 0.875f, 0.0f );
  glVertex2f( width - 13.0f, height - 13.0f );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( width + 3.0f, height - 13.0f );
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
  glVertex2f( width - 13.0f, 13.0f );  
  glTexCoord2f( 0.75f, 1.0f );
  glVertex2f( width + 3.0f, 13.0f );
  glTexCoord2f( 0.625f, 0.0f );
  glVertex2f( width - 13.0f, -3.0f );
  glTexCoord2f( 0.75f, 0.0f );
  glVertex2f( width + 3.0f, -3.0f );
  glEnd();

  //Draw Left Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( -3.0f, height - 13.0f );  
  glTexCoord2f( 0.125f, 1.0f );
  glVertex2f( 13.0f, height - 13.0f );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( -3.0f, 13.0f );
  glTexCoord2f( 0.125f, 0.0f );
  glVertex2f( 13, 13.0f );
  glEnd();

  //Draw Right Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.125f, 1.0f );
  glVertex2f( width - 13.0f, height - 13.0f );  
  glTexCoord2f( 0.25f, 1.0f );
  glVertex2f( width + 3.0f, height - 13.0f );
  glTexCoord2f( 0.125f, 0.0f );
  glVertex2f( width - 13.0f, 13.0f );
  glTexCoord2f( 0.25f, 0.0f );
  glVertex2f( width + 3.0f, 13.0f );
  glEnd();

  //Draw Top Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.5f, 1.0f );
  glVertex2f( 13.0f, height + 3.0f );  
  glTexCoord2f( 0.5f, 0.0f );
  glVertex2f( width - 13.0f, height + 3.0f );  
  glTexCoord2f( 0.375f, 1.0f );
  glVertex2f( 13, height - 13.0f );
  glTexCoord2f( 0.375f, 0.0f );
  glVertex2f( width - 13.0f, height - 13.0f );
  glEnd();

  //Draw Bottom Side
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.375f, 1.0f );
  glVertex2f( 13.0f, 13.0f );  
  glTexCoord2f( 0.375f, 0.0f );
  glVertex2f( width - 13.0f, 13.0f );
  glTexCoord2f( 0.25f, 1.0f );
  glVertex2f( 13.0f, -3.0f );
  glTexCoord2f( 0.25f, 0.0f );
  glVertex2f( width - 13.0f, -3.0f );
  glEnd();
  
  OpenGL::Texture::disableTexture();
  
  glPopMatrix();
}
