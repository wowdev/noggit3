#include "UIFrame.h"

#include <algorithm>
#include <vector>

#include "Video.h" // gl*

void UIFrame::render() const
{
  if( hidden )
    return;

  glPushMatrix();
  glTranslatef( x, y, 0.0f );

  for( std::vector<UIFrame*>::const_iterator child = children.begin(); child != children.end(); child++ )
    if( !( *child )->hidden )
      ( *child )->render();

  glPopMatrix();
}

void UIFrame::addChild( UIFrame *c )
{
  children.push_back( c );
  c->parent = this;
}

void UIFrame::removeChild( UIFrame* c )
{
  std::vector<UIFrame*>::iterator it = std::find( children.begin(), children.end(), c );
  if( it != children.end() )
  {
    children.erase( it );
  }
}


UIFrame * UIFrame::processLeftClick( float mx, float my )
{
  UIFrame * lTemp;
  for( std::vector<UIFrame*>::reverse_iterator child = children.rbegin(); child != children.rend(); child++ )
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

bool UIFrame::processLeftDrag( float /*mx*/, float /*my*/, float xDrag, float yDrag )
{
  if( movable )
  {
    x += xDrag;
    y += yDrag;
    return true;
  }

  return false;
}

bool UIFrame::processRightClick( float mx, float my )
{
  for( std::vector<UIFrame*>::iterator child = children.begin(); child != children.end(); child++ )
    if( !( *child )->hidden && ( *child )->IsHit( mx, my ) )
      if( ( *child )->processRightClick( mx - ( *child )->x, my - ( *child )->y ) )
        return true;

  return false;
}

void UIFrame::getOffset( float* xOff, float* yOff )
{
  float tx = 0.0f, ty = 0.0f;

  if( parent )
    parent->getOffset( &tx, &ty );

  *xOff = tx + x;
  *yOff = ty + y;
}

bool UIFrame::processKey( char /*key*/, bool /*shift*/, bool /*alt*/, bool /*ctrl*/ )
{
  return false;
}
  
void UIFrame::hide()
{
  hidden = true;
}
void UIFrame::show()
{
  hidden = false;
}
void UIFrame::toggleVisibility()
{
  hidden = !hidden;
}
