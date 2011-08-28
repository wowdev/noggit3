#include "UIGradient.h"

#include "Video.h" // gl*

void UIGradient::render() const
{
  if( hidden() )
  {
    return;
  }
    
  glPushMatrix();
  glTranslatef( x(), y(), 0.0f );
  
  if(horiz)
  {
    glBegin( GL_TRIANGLE_STRIP );
    glColor4fv( &MinColor.x );
    glVertex2f( 0.0f, 0.0f );    
    glVertex2f( 0.0f, height() );
    glColor4fv( &MaxColor.x );
    glVertex2f( width(), 0.0f );
    glVertex2f( width(), height() );
    glEnd();
    
    if( clickable() )
    {
      glColor4fv( &ClickColor.x );
      glBegin( GL_LINE );
      glVertex2f( width() * value, 0.0f );
      glVertex2f( width() * value, height() );
      glEnd();
    }
  }
  else
  {
    glBegin( GL_TRIANGLE_STRIP );
    glColor4fv( &MinColor.x );
    glVertex2f( width(), 0.0f );
    glVertex2f( 0.0f, 0.0f );
    glColor4fv( &MinColor.x );
    glVertex2f( width(), height() );
    glVertex2f( 0.0f, height() );
    glEnd();

    if( clickable() )
    {
      glColor4fv( &ClickColor.x );
      glBegin( GL_LINE );
      glVertex2f( 0.0f, height() * value );
      glVertex2f( width(), height() * value );
      glEnd();
    }
  }
  
  glPopMatrix();
}

void UIGradient::setMaxColor( float r, float g, float b, float a )
{
  MaxColor = Vec4D( r, g, b, a );
}

void UIGradient::setMinColor( float r, float g, float b, float a )
{
  MinColor = Vec4D( r, g, b, a );
}

void UIGradient::setClickColor( float r, float g, float b, float a )
{
  ClickColor = Vec4D( r, g, b, a );
}

void UIGradient::setClickFunc( void (*f)( float val ) )
{
  value = 0.0f;
  clickFunc = f;
  clickable( true );
}

UIFrame::Ptr UIGradient::processLeftClick( float mx, float my )
{
  if( clickable() )
  {
    if( horiz )
      value = mx / width();
    else
      value = my / height();
    
    value = std::min( std::max( value, 0.0f ), 1.0f );
    
    clickFunc( value );
    return this;
  }
  
  return NULL;
}

bool UIGradient::processLeftDrag( float mx, float my, float xDrag, float yDrag )
{
  float tx( 0.0f );
  float ty( 0.0f );
  
  parent()->getOffset( &tx, &ty );
  
  mx -= tx;
  my -= ty;
  
  if( processLeftClick( mx, my ) )
  {
    return true;
  }

  return UIFrame::processLeftDrag( mx, my, xDrag, yDrag );
}

void UIGradient::setValue( float f )
{
  value = std::min( std::max( f, 0.0f ), 1.0f );
  if( clickFunc )
  {
    clickFunc( value );
  }
}
