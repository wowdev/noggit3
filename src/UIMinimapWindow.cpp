#include <vector>

#include "Log.h"
#include "Menu.h"
#include "Sky.h"
#include "TextureManager.h"
#include "UIMinimapWindow.h"
#include "Video.h"
#include "World.h"

UIMinimapWindow::UIMinimapWindow( Menu* menuLink )
: UIWindow( 10.0f, 10.0f, 100.0f, 100.0f )
, borderwidth( 5.0f )
, tilesize( ( video.yres - 70.0f - this->borderwidth * 2.0f ) / 64.0f )
, lookAt( 0.0f )
, mMenuLink( menuLink )
, map( NULL )
{
  this->width = this->borderwidth * 2 + this->tilesize * 64;
  this->height = this->width;
  this->x = video.xres / 2 - this->width / 2;
  this->y = video.yres / 2 - this->height / 2;
}

UIMinimapWindow::UIMinimapWindow( World* setMap )
: UIWindow( 10.0f, 10.0f, 100.0f, 100.0f )
, borderwidth( 5.0f )
, tilesize( ( video.yres - 70.0f - this->borderwidth * 2.0f ) / 64.0f )
, lookAt( 0.0f )
, mMenuLink( NULL )
, map( setMap )
{
  this->width = this->borderwidth * 2 + this->tilesize * 64;
  this->height = this->width;
  this->x = video.xres / 2 - this->width / 2;
  this->y = video.yres / 2 - this->height / 2;
}

UIFrame* UIMinimapWindow::processLeftClick( float mx, float my )
{
  // no click outside the adt block
  if( !gWorld || !mMenuLink ||
      mx < this->borderwidth || mx > this->height - this->borderwidth ||
      my < this->borderwidth || my > this->height - this->borderwidth )
    return NULL;

  // is there a tile?
  int i = static_cast<int>( mx - this->borderwidth ) / this->tilesize;
  int j = static_cast<int>( my - this->borderwidth ) / this->tilesize;
  if( !gWorld->hasTile(j,i) ) 
    return NULL;    

  mMenuLink->enterMapAt( Vec3D( ( ( mx - this->borderwidth ) / this->tilesize ) * TILESIZE, 0.0f, ( ( my - this->borderwidth ) / this->tilesize ) * TILESIZE ) );
  
  return this;
}


void UIMinimapWindow::resize()
{
  this->tilesize = (video.yres - 70.0f - this->borderwidth * 2) / 64;

  this->width = this->borderwidth * 2 + this->tilesize * 64;
  this->height = this->width;
  this->x = video.xres / 2 - this->width / 2;
  this->y = video.yres / 2 - this->height / 2;
}

void UIMinimapWindow::changePlayerLookAt(float ah)
{
  this->lookAt = ah;
}

void UIMinimapWindow::render() const
{
  UIWindow::render();
  
  glPushMatrix();
  glTranslatef( x + this->borderwidth, y + this->borderwidth, 0.0f );
  
  if( gWorld->minimap ) 
  {
    OpenGL::Texture::enableTexture();
    glBindTexture( GL_TEXTURE_2D, gWorld->minimap );
    
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2i( 0.0f, 0.0f );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2i( this->tilesize * 64.0f, 0.0f );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2i( this->tilesize * 64.0f, this->tilesize * 64.0f );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2i( 0.0f, this->tilesize * 64.0f );
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
      if( gWorld->hasTile(j,i) ) 
        glColor4f( 0.8f, 0.8f, 0.8f, 0.4f );
      else
        glColor4f( 1.0f, 1.0f, 1.0f, 0.05f );
      
      glBegin( GL_QUADS );
      glVertex2i( 0.0f + i * this->tilesize, 0.0f + j * this->tilesize );
      glVertex2i( (0.0f + ( i + 1 ) * this->tilesize) - 1, 0.0f + j * this->tilesize );
      glVertex2i( (0.0f + ( i + 1 ) * this->tilesize) - 1, (0.0f + ( j + 1 ) * this->tilesize) -1 );
      glVertex2i( 0.0f + i * this->tilesize, (0.0f + ( j + 1 ) * this->tilesize) -1 );
      glEnd();

      if( this->map )
        if( this->map->getChanged(j,i) )
        {
          glColor4f( 1.0f, 1.0f, 1.0f, 0.6f );
          glBegin( GL_LINES );
          glVertex2i( 0.0f + i * this->tilesize, 0.0f + j * this->tilesize );
          glVertex2i( (0.0f + ( i + 1 ) * this->tilesize) , 0.0f + j * this->tilesize );
          glVertex2i( (0.0f + ( i + 1 ) * this->tilesize) , 0.0f + j * this->tilesize );
          glVertex2i( (0.0f + ( i + 1 ) * this->tilesize) , (0.0f + ( j + 1 ) * this->tilesize) -1 );
          glVertex2i( (0.0f + ( i + 1 ) * this->tilesize) , (0.0f + ( j + 1 ) * this->tilesize) -1 );
          glVertex2i( 0.0f + i * this->tilesize, (0.0f + ( j + 1 ) * this->tilesize) -1 );
          glVertex2i( 0.0f + i * this->tilesize, (0.0f + ( j + 1 ) * this->tilesize) -1 );
          glVertex2i( 0.0f + i * this->tilesize, 0.0f + j * this->tilesize );
          glEnd();

        }

    }
  }
  
  // draw the arrow if shown inside a map
  //! \todo Change it from a simple line to an arrow
  if( this->map )
  {
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    glBegin(GL_LINES);
    float fx, fz;
    fx = map->camera.x / TILESIZE * this->tilesize;
    fz = map->camera.z / TILESIZE * this->tilesize;
    glVertex2f(fx, fz);
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    glVertex2f(fx + 10.0f*cosf(this->lookAt/180.0f*PI), fz + 10.0f*sinf(this->lookAt/180.0f*PI));
    glEnd();

    int skycount = this->map->skies->skies.size();

    
    for( int j = 0; j < skycount; j++ )
    {
      glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
      float x_ = this->map->skies->skies[j].pos.x/ TILESIZE * this->tilesize;
      float z_ = this->map->skies->skies[j].pos.z/ TILESIZE * this->tilesize;
      glBegin( GL_QUADS );
      glVertex2i( 0.0f + x_,  0.0f + z_ );
      glVertex2i( 0.0f + x_+3,  0.0f + z_ );
      glVertex2i( 0.0f + x_+3,  0.0f + z_+3 );
      glVertex2i( 0.0f + x_,  0.0f + z_+3 );
      glEnd();
    }
  }

  glPopMatrix();
}