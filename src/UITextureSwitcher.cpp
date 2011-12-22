#include "UITextureSwitcher.h"

#include "World.h"
#include "Selection.h"
#include "MapChunk.h"
#include "UITexture.h"
#include "UITexturingGUI.h"

void textureSwitcherClick( UIFrame* f,int id )
{
  // redirect to sender object.
  ( static_cast<UITextureSwitcher *>( f->parent() ) )->setTexture( id );
}


UITextureSwitcher::UITextureSwitcher( int x, int y )
: UICloseWindow( x, y, 130, 140, "Texture", true )
{
   const int textureSize = 110;

	_textures = new UITexture( 10 , 25, textureSize, textureSize, "tileset\\generic\\black.blp" );

    addChild( _textures );
  
}

void UITextureSwitcher::getTextures( nameEntry* lSelection )
{
  assert( lSelection );

  show();

 
}

void UITextureSwitcher::setTexture( size_t id )
{
  assert( id < 4 );
  //gWorld->overwriteTextureAtCurrentChunk( this->xPos, this->zPos, _textures, UITexturingGUI::getSelectedTexture());
}

void UITextureSwitcher::setPosition( int setX, int setY )
{
	this->xPos = setX;
	this->zPos = setY;
}

