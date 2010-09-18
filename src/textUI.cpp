#include "textUI.h"

textUI::textUI( float pX, float pY, const std::string& pText, freetype::font_data *pFont, int pJustify ) : background( false )
{
	x = pX;
	y = pY;
	mText = pText;
	justify = pJustify;
	font = pFont;

	twidth = freetype::width( *font, mText.c_str( ) );
}

textUI::textUI( float pX, float pY, freetype::font_data *pFont, int pJustify ) : background( false )
{
	x = pX;
	y = pY;
	mText = "";
	justify = pJustify;
	font = pFont;
	
	twidth = freetype::width( *font, mText.c_str( ) );
}


void textUI::setText( const std::string& pText )
{
	mText = pText;
	twidth = freetype::width( *font, mText.c_str( ) );
}

void textUI::setJustify(int j)
{
	justify = j;
}

void textUI::setFont( freetype::font_data *f )
{
	font = f;
	twidth = freetype::width( *font, mText.c_str( ) );
}

void textUI::render( )
{
	width = twidth;
	height = font->h;

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
		glEnd( );
	}

	switch( justify )
	{
	case eJustifyLeft:
		freetype::shprint( *font, x, y, mText.c_str( ) );		
		break;
	case eJustifyCenter:
		freetype::shprint( *font, x - twidth / 2.0f, y, mText.c_str( ) );
		break;
	case eJustifyRight:
		freetype::shprint( *font, x - twidth, y, mText.c_str( ) );
		break;
	}
}

void textUI::setBackground( float r, float g, float b, float a )
{
	background = true;
	bgColor[0] = r;
	bgColor[1] = g;
	bgColor[2] = b;
	bgColor[3] = a;
}
