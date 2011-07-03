#include "UIText.h"

#include <string>

#include "FreeType.h"
#include "Video.h"

UIText::UIText( float pX, float pY, const std::string& pText, freetype::font_data *pFont, int pJustify )
: UIFrame( pX, pY, freetype::width( *pFont, pText ), pFont->h )
, twidth( freetype::width( *pFont, pText ) )
, font( pFont )
, mText( pText )
, justify( pJustify )
, background( false )
{
}

UIText::UIText( float pX, float pY, freetype::font_data *pFont, int pJustify )
: UIFrame( pX, pY, freetype::width( *pFont, "" ), pFont->h )
, twidth( freetype::width( *pFont, "" ) )
, font( pFont )
, mText( "" )
, justify( pJustify )
, background( false )
{
}

void UIText::setText( const std::string& pText )
{
	mText = pText;
	twidth = freetype::width( *font, mText );
	width = twidth;
	height = font->h;
}

void UIText::setJustify( int j )
{
	justify = j;
}

void UIText::setFont( freetype::font_data* f )
{
	font = f;
	twidth = freetype::width( *font, mText );
	width = twidth;
	height = font->h;
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
			glVertex2f( x - 2.0f, y - 1.0f);
			glVertex2f( x + 2.0f + twidth, y - 1.0f );
			glVertex2f( x - 2.0f, y + font->h + 3.0f );
			glVertex2f( x + 2.0f + twidth, y + font->h + 3.0f );
			break;
		case eJustifyCenter:
			glVertex2f( x - 2.0f - twidth / 2.0f, y - 1.0f );
			glVertex2f( x + 2.0f + twidth / 2.0f, y - 1.0f );
			glVertex2f( x - 2.0f - twidth / 2.0f, y + font->h + 3.0f );
			glVertex2f( x + 2.0f + twidth / 2.0f, y + font->h + 3.0f );	
			break;
		case eJustifyRight:
			glVertex2f( x - 2.0f - twidth, y - 1.0f );
			glVertex2f( x + 2.0f, y - 1.0f );
			glVertex2f( x - 2.0f - twidth,y + font->h + 3.0f);
			glVertex2f( x + 2.0f, y + font->h + 3.0f);	
			break;
		}
		glEnd();
	}
  
	switch( justify )
	{
	case eJustifyLeft:
		freetype::shprint( *font, x, y, mText );		
		break;
	case eJustifyCenter:
		freetype::shprint( *font, x - twidth / 2.0f, y, mText );
		break;
	case eJustifyRight:
		freetype::shprint( *font, x - twidth, y, mText );
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
