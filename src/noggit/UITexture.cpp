// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/UITexture.h>

#include <string>

#include <noggit/blp_texture.h>
#include <noggit/TextureManager.h> // TextureManager

#include <opengl/context.hpp>

UITexture::UITexture( float xPos, float yPos, float w, float h, const std::string& tex )
: UIFrame( xPos, yPos, w, h )
, texture( TextureManager::newTexture( tex ) )
, _textureFilename( tex )
, highlight( false )
, clickFunc( nullptr )
, id( 0 )
{
}

UITexture::~UITexture()
{
  if( texture )
  {
    TextureManager::delbyname( _textureFilename );
    texture = nullptr;
  }
}

void UITexture::setTexture( noggit::blp_texture* tex )
{
  //! \todo Free current texture.
  //! \todo New reference?
  texture = tex;
}

void UITexture::setTexture( const std::string& textureFilename )
{
  if( texture )
  {
    TextureManager::delbyname( _textureFilename );
    texture = nullptr;
  }
  _textureFilename = textureFilename;
  texture = TextureManager::newTexture( textureFilename );
}

noggit::blp_texture* UITexture::getTexture( )
{
  return texture;
}

void UITexture::render() const
{
  gl.pushMatrix();
  gl.translatef( x(), y(), 0.0f );

  gl.color3f( 1.0f, 1.0f, 1.0f );

  opengl::texture::enable_texture (0);

  texture->bind();

  gl.begin( GL_TRIANGLE_STRIP );
  gl.texCoord2f( 0.0f, 0.0f );
  gl.vertex2f( 0.0f, 0.0f );
  gl.texCoord2f( 1.0f, 0.0f );
  gl.vertex2f( width(), 0.0f );
  gl.texCoord2f( 0.0f, 1.0f );
  gl.vertex2f( 0.0f, height() );
  gl.texCoord2f( 1.0f, 1.0f );
  gl.vertex2f( width(), height() );
  gl.end();

  opengl::texture::disable_texture (0);

  if( highlight )
  {
    gl.color3f( 1.0f, 0.0f, 0.0f );
    gl.begin( GL_LINE_LOOP );
    gl.vertex2f( -1.0f, 0.0f );
    gl.vertex2f( width(), 0.0f );
    gl.vertex2f( width(), height() );
    gl.vertex2f( -1.0f, height() );
    gl.end();
  }

  gl.popMatrix();
}

UIFrame *UITexture::processLeftClick( float /*mx*/, float /*my*/ )
{
  if( clickFunc )
  {
    clickFunc( this, id );
    return this;
  }
  return 0;
}

void UITexture::setClickFunc( void (*f)( UIFrame *, int ), int num )
{
  clickFunc = f;
  id = num;
}
