#include "frame.h"

#include <algorithm>
#include <vector>

#include "video.h" // gl*

void frame::render() const
{
  if( hidden )
    return;

  glPushMatrix();
  glTranslatef( x, y, 0.0f );

  for( std::vector<frame*>::const_iterator child = children.begin(); child != children.end(); child++ )
    if( !( *child )->hidden )
      ( *child )->render();

  glPopMatrix();
}

void frame::addChild( frame *c )
{
  children.push_back( c );
  c->parent = this;
}

void frame::removeChild( frame* c )
{
  std::vector<frame*>::iterator it = std::find( children.begin(), children.end(), c );
  if( it != children.end() )
  {
    children.erase( it );
  }
}


frame * frame::processLeftClick( float mx, float my )
{
  frame * lTemp;
  for( std::vector<frame*>::reverse_iterator child = children.rbegin(); child != children.rend(); child++ )
  {
    if( !( *child )->hidden && ( *child )->IsHit( mx, my ) )
    {
      lTemp = ( *child )->processLeftClick( mx - ( *child )->x, my - ( *child )->y );
      if( lTemp )
        return lTemp;
    }
  }
  return 0;
}

bool frame::processLeftDrag( float /*mx*/, float /*my*/, float xDrag, float yDrag )
{
  if( movable )
  {
    x += xDrag;
    y += yDrag;
    return true;
  }

  return false;
}

bool frame::processRightClick( float mx, float my )
{
  for( std::vector<frame*>::iterator child = children.begin(); child != children.end(); child++ )
    if( !( *child )->hidden && ( *child )->IsHit( mx, my ) )
      if( ( *child )->processRightClick( mx - ( *child )->x, my - ( *child )->y ) )
        return true;

  return false;
}

void frame::getOffset( float* xOff, float* yOff )
{
  float tx = 0.0f, ty = 0.0f;

  if( parent )
    parent->getOffset( &tx, &ty );

  *xOff = tx + x;
  *yOff = ty + y;
}

bool frame::processKey( char /*key*/, bool /*shift*/, bool /*alt*/, bool /*ctrl*/ )
{
  return false;
}
