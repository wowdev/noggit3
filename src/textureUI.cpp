#include "textureUI.h"

textureUI::textureUI( float xPos, float yPos, float w, float h, GLuint tex ) : frame( xPos, yPos, w, h )
{
	texture = tex;
	highlight = false;
	clickFunc = 0;
	id = 0;
}

textureUI::textureUI( float xPos, float yPos, float w, float h, const std::string& tex ) : frame( xPos, yPos, w, h )
{
	texture = video.textures.add( tex );
	highlight = false;
	clickFunc = 0;
	id = 0;
}

void textureUI::setTexture( GLuint tex )
{
	texture = tex;
}

void textureUI::setTexture( const std::string& tex )
{
	texture = video.textures.add( tex );
}

void textureUI::render( )
{
	glColor3f( 1.0f, 1.0f, 1.0f );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, texture );
	glEnable( GL_TEXTURE_2D );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0.0f, 0.0f );
	glVertex2f( x, y );
	glTexCoord2f( 1.0f, 0.0f );
	glVertex2f( x + width, y );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex2f( x, y + height );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex2f( x + width, y + height );
	glEnd( );

	glDisable( GL_TEXTURE_2D );

	if( highlight )
	{
		glColor3f( 1.0f, 0.0f, 0.0f );
		glBegin( GL_LINE_LOOP );
		glVertex2f( x - 1.0f, y );
		glVertex2f( x + width, y );
		glVertex2f( x + width, y + height );
		glVertex2f( x - 1.0f, y + height );
		glEnd( );
	}
}

frame *textureUI::processLeftClick( float mx, float my )
{
	if( clickFunc )
	{
		clickFunc( this, id );
		return this;
	}
	return 0;
}

void textureUI::setClickFunc( void (*f)( frame *, int ), int num )
{
	clickFunc = f;
	id = num;
}
