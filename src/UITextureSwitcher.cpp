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

UITextureSwitcher::UITextureSwitcher( World* world, float x, float y, float w, float h )
  : UICloseWindow( x, y, w, h, "Select the texture you want to switch with the one currently selected.", true )
  , _world (world)
{
  const int textureSize = 110;
  const int startingX = 10;
  const int paddingX = 10;
  const int positionY = 30;

  for( size_t i = 0; i < 4; ++i )
  {
    _textures[i] = new UITexture( startingX + ( textureSize + paddingX ) * i, positionY, textureSize, textureSize, "tileset\\generic\\black.blp" );
    _textures[i]->setClickFunc( textureSwitcherClick, i );
    addChild( _textures[i] );
  }
}

void UITextureSwitcher::getTextures( nameEntry* lSelection )
{
  assert( lSelection );

  show();

  if( lSelection->type == eEntry_MapChunk )
  {
  lSelection->data.mapchunk->getSelectionCoord(&this->xPos, &this->zPos);
    MapChunk* chunk = lSelection->data.mapchunk;

    size_t index = 0;

    for( ; index < 4U && chunk->nTextures > index; ++index )
    {
      _textures[index]->setTexture( chunk->_textures[index] );
      _textures[index]->show();
    }

    for( ; index < 4U; ++index )
    {
      _textures[index]->hide();
    }
  }
}

void UITextureSwitcher::setTexture( size_t id )
{
  assert( id < 4 );
  _world->overwriteTextureAtCurrentChunk( this->xPos, this->zPos, _textures[id]->getTexture(), UITexturingGUI::getSelectedTexture());
}
