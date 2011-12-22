#include "UIFrame.h"

#include <SDL.h>
#include <algorithm>
#include <vector>

#include "Video.h" // gl*

void UIFrame::render() const
{
  if( hidden() )
    return;

  glPushMatrix();
  glTranslatef( x(), y(), 0.0f );

  renderChildren();

  glPopMatrix();
}

void UIFrame::renderChildren() const
{
  for( Children::const_iterator child( children().begin() ), end( children().end() )
    ; child != end; ++child )
  {
    if( !( *child )->hidden() )
    {
      ( *child )->render();
    }
  }
}

void UIFrame::addChild( UIFrame::Ptr c )
{
  _children.push_back( c );
  c->parent( this );
}

void UIFrame::removeChild( UIFrame::Ptr c )
{
  Children::iterator end( _children.end() );
  Children::iterator it( std::find( _children.begin(), end, c ) );
  if( it != end )
  {
    _children.erase( it );
  }
}


UIFrame::Ptr UIFrame::processLeftClick( float mx, float my )
{
  UIFrame::Ptr lTemp;
  for( Children::reverse_iterator child( _children.rbegin()), end( _children.rend() )
     ; child != end; ++child )
  {
    if( !( *child )->hidden() && ( *child )->IsHit( mx, my ) )
    {
      lTemp = ( *child )->processLeftClick( mx - ( *child )->x(), my - ( *child )->y() );
      if( lTemp )
        return lTemp;
    }
  }
  return NULL;
}

bool UIFrame::processLeftDrag( float /*mx*/, float /*my*/, float xDrag, float yDrag )
{
  if( movable() )
  {
    _x += xDrag;
    _y += yDrag;
    return true;
  }

  return false;
}

bool UIFrame::processRightClick( float mx, float my )
{
  for( Children::iterator child( _children.begin()), end( _children.end() )
     ; child != end; ++child )
  {
    if( !( *child )->hidden() && ( *child )->IsHit( mx, my ) )
    {
      if( ( *child )->processRightClick( mx - ( *child )->x(), my - ( *child )->y() ) )
      {
        return true;
      }
    }
  }

  return false;
}

void UIFrame::getOffset( float* xOff, float* yOff )
{
  float tx( 0.0f );
  float ty( 0.0f );

  if( parent() )
  {
    parent()->getOffset( &tx, &ty );
  }

  *xOff = tx + x();
  *yOff = ty + y();
}

bool UIFrame::KeyBoardEvent( SDL_KeyboardEvent *e )
{
  return false;
}

int UIFrame::getX()
{
	return this->_x;
}

int UIFrame::getY()
{
	return this->_y;
}

void UIFrame::setX(int setX)
{
	this->_x = setX;
}

void UIFrame::setY(int setY)
{
	this->_y = setY;
}