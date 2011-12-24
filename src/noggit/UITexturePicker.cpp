#include <noggit/UITexturePicker.h>

#include <noggit/Selection.h>
#include <noggit/MapChunk.h>
#include <noggit/UITexture.h>
#include <noggit/UITexturingGUI.h>

void texturePickerClick( UIFrame* f,int id )
{
  // redirect to sender object.
  ( static_cast<UITexturePicker *>( f->parent() ) )->setTexture( id );
}

UITexturePicker::UITexturePicker( float x, float y, float w, float h )
: UICloseWindow( x, y, w, h, "Pick one of the textures.", true )
{
  const int textureSize = 110;
  const int startingX = 10;
  const int paddingX = 10;
  const int positionY = 30;

  for( size_t i = 0; i < 4; ++i )
  {
    _textures[i] = new UITexture( startingX + ( textureSize + paddingX ) * i, positionY, textureSize, textureSize, "tileset\\generic\\black.blp" );
    _textures[i]->setClickFunc( texturePickerClick, i );
    addChild( _textures[i] );
  }
}

void UITexturePicker::getTextures( nameEntry* lSelection )
{
  assert( lSelection );

  show();

  if( lSelection->type == eEntry_MapChunk )
  {
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

void UITexturePicker::setTexture( size_t id )
{
  assert( id < 4 );

  UITexturingGUI::setSelectedTexture( _textures[id]->getTexture() );
  UITexturingGUI::updateSelectedTexture();
}
