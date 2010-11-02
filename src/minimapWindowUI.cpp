#include "minimapWindowUI.h"
#include "video.h"
#include "TextureManager.h"
#include "world.h"
#include "menu.h"

const float minimapWindowUI::tilesize = 12.0f;

minimapWindowUI::minimapWindowUI( Menu* menuLink, float px, float py ) : window( px, py, tilesize * 66, tilesize * 66 ), mMenuLink( menuLink )
{
}

frame* minimapWindowUI::processLeftClick( float mx, float my )
{
	if( !mMenuLink ||
			mx < tilesize || mx > this->height - tilesize ||
			my < tilesize || mx > this->height - tilesize )
		return NULL;
	
	mMenuLink->enterMapAt( Vec3D( ( ( mx - 12.0f ) / 12.0f ) * TILESIZE, 0.0f, ( ( my - 12.0f ) / 12.0f ) * TILESIZE ) );
	
	return this;
}

void minimapWindowUI::render() const
{
	window::render();
	
	glPushMatrix();
	glTranslatef( x + tilesize, y + tilesize, 0.0f );
	
	if( gWorld->minimap ) 
	{
		Texture::enableTexture();
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
		
		Texture::disableTexture();
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
			{
				glColor4f( 0.8f, 0.8f, 0.8f, 0.4f );
			}
			else
			{
				glColor4f( 1.0f, 1.0f, 1.0f, 0.05f );
			}
			
			glBegin( GL_QUADS );
			glVertex2i( 0.0f + i * tilesize, 0.0f + j * tilesize );
			glVertex2i( (0.0f + ( i + 1 ) * tilesize) - 1, 0.0f + j * tilesize );
			glVertex2i( (0.0f + ( i + 1 ) * tilesize) - 1, (0.0f + ( j + 1 ) * tilesize) -1 );
			glVertex2i( 0.0f + i * tilesize, (0.0f + ( j + 1 ) * tilesize) -1 );
			glEnd();
		}
	}
	
	glPopMatrix();
}