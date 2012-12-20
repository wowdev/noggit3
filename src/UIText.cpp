#include "UIText.h"

#include <string>

#include "FreeType.h"
#include "Video.h"
#include "Log.h"

UIText::UIText( float pX, float pY, const std::string& pText, const freetype::font_data& pFont, eJustify pJustify )
: UIFrame( pX, pY, pFont.width( pText ), pFont.h )
, font( pFont )
, mText( pText )
, justify( pJustify )
, background( false )
{
}

UIText::UIText( float pX, float pY, const freetype::font_data& pFont, eJustify pJustify )
: UIFrame( pX, pY, 0, pFont.h )
, font( pFont )
, mText( "" )
, justify( pJustify )
, background( false )
{
}

void UIText::setText( const std::string& pText )
{
  mText = pText;
  width( font.width( mText ) );
}

void UIText::setJustify( eJustify j )
{
  justify = j;
}

void UIText::render() const
{
  if( background )
  {
    glColor4fv( bgColor );
    glBegin( GL_TRIANGLE_STRIP );
    switch( justify )
    {
    case eJustifyLeft:
      glVertex2f( x() - 2.0f, y() - 1.0f);
      glVertex2f( x() + 2.0f + width(), y() - 1.0f );
      glVertex2f( x() - 2.0f, y() + font.h + 3.0f );
      glVertex2f( x() + 2.0f + width(), y() + font.h + 3.0f );
      break;
    case eJustifyCenter:
      glVertex2f( x() - 2.0f - width() / 2.0f, y() - 1.0f );
      glVertex2f( x() + 2.0f + width() / 2.0f, y() - 1.0f );
      glVertex2f( x() - 2.0f - width() / 2.0f, y() + font.h + 3.0f );
      glVertex2f( x() + 2.0f + width() / 2.0f, y() + font.h + 3.0f );
      break;
    case eJustifyRight:
      glVertex2f( x() - 2.0f - width(), y() - 1.0f );
      glVertex2f( x() + 2.0f, y() - 1.0f );
      glVertex2f( x() - 2.0f - width(),y() + font.h + 3.0f);
      glVertex2f( x() + 2.0f, y() + font.h + 3.0f);
      break;
    }
    glEnd();
  }

  switch( justify )
  {
  case eJustifyLeft:
    font.shprint( x(), y(), mText );
    break;
  case eJustifyCenter:
    font.shprint( x() - width() / 2.0f, y(), mText );
    break;
  case eJustifyRight:
    font.shprint( x() - width(), y(), mText );
    break;
  }
}

void UIText::setBackground( float r, float g, float b, float a )
{
  background = true;
  bgColor[0] = r;
  bgColor[1] = g;
  bgColor[2] = b;
  bgColor[3] = a;
}
