#include "textureUI.h"
#include "TextureManager.h" // TextureManager, Texture

/*textureUI::textureUI( float xPos, float yPos, float w, float h, GLuint tex ) : frame( xPos, yPos, w, h )
{
	texture = tex;
	highlight = false;
	clickFunc = 0;
	id = 0;
}*/

textureUI::textureUI( float xPos, float yPos, float w, float h, const std::string& tex ) : frame( xPos, yPos, w, h )
{
	texture = TextureManager::newTexture( tex );
	highlight = false;
	clickFunc = 0;
	id = 0;
}

void textureUI::setTexture( GLuint tex )
{
	//! \todo Free current texture.
	texture = (OpenGL::Texture*)TextureManager::items[tex];
}

void textureUI::setTexture( const std::string& tex )
{
	texture = TextureManager::newTexture( tex );
}

void textureUI::render() const
{
	glColor3f( 1.0f, 1.0f, 1.0f );

	OpenGL::Texture::setActiveTexture();
	OpenGL::Texture::enableTexture();
	
	texture->render();

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0.0f, 0.0f );
	glVertex2f( x, y );
	glTexCoord2f( 1.0f, 0.0f );
	glVertex2f( x + width, y );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex2f( x, y + height );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex2f( x + width, y + height );
	glEnd();
	
	OpenGL::Texture::disableTexture();

	if( highlight )
	{
		glColor3f( 1.0f, 0.0f, 0.0f );
		glBegin( GL_LINE_LOOP );
		glVertex2f( x - 1.0f, y );
		glVertex2f( x + width, y );
		glVertex2f( x + width, y + height );
		glVertex2f( x - 1.0f, y + height );
		glEnd();
	}
}

frame *textureUI::processLeftClick( float /*mx*/, float /*my*/ )
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
