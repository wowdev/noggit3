#include <vector>

#include "Log.h"
#include "Menu.h"
#include "Sky.h"
#include "UIMinimapWindow.h"
#include "Video.h"
#include "World.h"

UIMinimapWindow::UIMinimapWindow( Menu* menuLink )
: UIWindow( 0.0f, 0.0f, 0.0f, 0.0f )
, borderwidth( 5.0f )
, tilesize( 0.0f )
, lookAt( 0.0f )
, mMenuLink( menuLink )
, map( NULL )
{
  resize();
}

UIMinimapWindow::UIMinimapWindow( World* setMap )
: UIWindow( 0.0f, 0.0f, 0.0f, 0.0f )
, borderwidth( 5.0f )
, tilesize( 0.0f )
, lookAt( 0.0f )
, mMenuLink( NULL )
, map( setMap )
{
  resize();
}

UIFrame* UIMinimapWindow::processLeftClick( float mx, float my )
{
  // no click outside the adt block
  if( !gWorld || !mMenuLink ||
      mx < borderwidth || mx > height() - borderwidth ||
      my < borderwidth || my > height() - borderwidth )
    return NULL;

  // is there a tile?
  int i = static_cast<int>( mx - borderwidth ) / tilesize;
  int j = static_cast<int>( my - borderwidth ) / tilesize;
  if( !gWorld->hasTile( j, i ) ) 
    return NULL;    

  mMenuLink->enterMapAt( Vec3D( ( ( mx - borderwidth ) / tilesize ) * TILESIZE, 0.0f, ( ( my - borderwidth ) / tilesize ) * TILESIZE ) );
  
  return this;
}


void UIMinimapWindow::resize()
{
  tilesize = ( video.yres() - 70.0f - borderwidth * 2.0f ) / 64.0f;

  width( borderwidth * 2.0f + tilesize * 64.0f );
  height( width() );
  x( video.xres() / 2.0f - width() / 2.0f );
  y( video.yres() / 2.0f - height() / 2.0f );
}

void UIMinimapWindow::changePlayerLookAt(float ah)
{
  lookAt = ah;
}

void UIMinimapWindow::render() const
{
  if( hidden() )
  {
    return;
  }
  
  UIWindow::render();
  
  glPushMatrix();
  glTranslatef( x() + borderwidth, y() + borderwidth, 0.0f );
  
  if( gWorld->minimap ) 
  {
    OpenGL::Texture::enableTexture();
    glBindTexture( GL_TEXTURE_2D, gWorld->minimap );
    
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2i( 0.0f, 0.0f );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2i( tilesize * 64.0f, 0.0f );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2i( tilesize * 64.0f, tilesize * 64.0f );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2i( 0.0f, tilesize * 64.0f );
    glEnd();
    
    OpenGL::Texture::disableTexture();
  }
  
  // draw the ADTs that are existing in the WDT with
  // a transparent 11x11 box. 12x12 is the full size 
  // so we get a small border. Draw all not existing adts with 
  // a lighter box to have all 64x64 possible adts on screen. 
  // Later we can create adts over this view ore move them.
  for( int j = 0; j < 64; j++ ) 
  {
    for( int i = 0; i < 64; ++i ) 
    {
      if( gWorld->hasTile( j, i ) ) 
        glColor4f( 0.8f, 0.8f, 0.8f, 0.4f );
      else
        glColor4f( 1.0f, 1.0f, 1.0f, 0.05f );
      
      glBegin( GL_QUADS );
      glVertex2i( i * tilesize, j * tilesize );
      glVertex2i( (( i + 1 ) * tilesize) - 1, j * tilesize );
      glVertex2i( (( i + 1 ) * tilesize) - 1, (( j + 1 ) * tilesize) -1 );
      glVertex2i( i * tilesize, (( j + 1 ) * tilesize) -1 );
      glEnd();

      if( map )
      {
        if( map->getChanged(j,i) )
        {
          glColor4f( 1.0f, 1.0f, 1.0f, 0.6f );
          glBegin( GL_LINES );
          glVertex2i( i * tilesize, j * tilesize );
          glVertex2i( (( i + 1 ) * tilesize) , j * tilesize );
          glVertex2i( (( i + 1 ) * tilesize) , j * tilesize );
          glVertex2i( (( i + 1 ) * tilesize) , (( j + 1 ) * tilesize) -1 );
          glVertex2i( (( i + 1 ) * tilesize) , (( j + 1 ) * tilesize) -1 );
          glVertex2i( i * tilesize, (( j + 1 ) * tilesize) -1 );
          glVertex2i( i * tilesize, (( j + 1 ) * tilesize) -1 );
          glVertex2i( i * tilesize, j * tilesize );
          glEnd();

        }
      }
    }
  }
  
  // draw the arrow if shown inside a map
  //! \todo Change it from a simple line to an arrow
  if( map )
  {
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    glBegin( GL_LINES );
    const float fx( map->camera.x / TILESIZE * tilesize );
    const float fz( map->camera.z / TILESIZE * tilesize );
    glVertex2f( fx, fz );
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    glVertex2f( fx + 10.0f * cosf( lookAt / 180.0f * PI ), fz + 10.0f * sinf( lookAt / 180.0f * PI ) );
    glEnd();

    int skycount = map->skies->skies.size();

    
    for( int j = 0; j < skycount; j++ )
    {
      glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
      float x_ = map->skies->skies[j].pos.x/ TILESIZE * tilesize;
      float z_ = map->skies->skies[j].pos.z/ TILESIZE * tilesize;
      glBegin( GL_QUADS );
      glVertex2i( x_,  z_ );
      glVertex2i( x_+3,  z_ );
      glVertex2i( x_+3,  z_+3 );
      glVertex2i( x_,  z_+3 );
      glEnd();
    }
  }

  glPopMatrix();
}