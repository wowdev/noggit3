#include "FreeType.h"

#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H

#include <algorithm>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <utf8.h>

#include "MPQ.h"
#include "Log.h"
#include "Video.h"

namespace freetype
{
  inline int next_p2( int a )
  {
    int rval( 1 );
    while( rval < a )
    {
      rval <<= 1;
    }
    return rval;
  }

  void font_data::createGlyph( CharacterCode charCode ) const
  {
    if( FT_Load_Glyph( _face, FT_Get_Char_Index( _face, charCode ), FT_LOAD_DEFAULT ) )
    {
      LogError << "FT_Load_Glyph failed" << std::endl;
      throw std::runtime_error("FT_Load_Glyph failed");
    }

    FT_Glyph glyph;
    if( FT_Get_Glyph( _face->glyph, &glyph ) )
    {
      LogError << "FT_Get_Glyph failed" << std::endl;
      throw std::runtime_error("FT_Get_Glyph failed");
    }

    FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 );
    FT_BitmapGlyph bitmap_glyph( reinterpret_cast<FT_BitmapGlyph>( glyph ) );

    const FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    const int width( next_p2( bitmap.width + 1 ) );
    const int height( next_p2( bitmap.rows + 1 ) );

    const size_t expanded_size( width * height* 2 );
    GLubyte* expanded_data = new GLubyte[expanded_size];
    memset( expanded_data, 0, expanded_size );

    for( int j( 0 ); j < height; ++j )
    {
      for( int i( 0 ); i < width; ++i )
      {
        expanded_data[ 2 * ( i + j * width ) ] = expanded_data[ 2 * ( i + j * width ) + 1 ] =
          ( i >= bitmap.width || j >= bitmap.rows ) ?
          0 : static_cast<GLubyte>( std::min( static_cast<float>( bitmap.buffer[ i + bitmap.width * j ] ) * 1.5f, 255.0f ) );
      }
    }

    GlyphData glyphData;
    glyphData._width = _face->glyph->advance.x >> 6;

    glyphData._texture = new OpenGL::Texture();
    glyphData._texture->bind();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

    delete[] expanded_data;
    expanded_data = NULL;

    glyphData._callList = new OpenGL::CallList();
    glyphData._callList->startRecording();

    glyphData._texture->bind();

    const float xl( bitmap_glyph->left );
    const float xh( xl + width );
    const float yl( h - bitmap_glyph->top );
    const float yh( yl + height );

    glBegin( GL_TRIANGLE_STRIP );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( xl, yl );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2f( xl, yh );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2f( xh, yl );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2f( xh, yh );
    glEnd();

    glTranslatef( static_cast<float>( glyphData._width ), 0.0f, 0.0f );

    glyphData._callList->endRecording();

    _cachedGlyphs[charCode] = glyphData;
  }

  void font_data::init( const std::string& fname, unsigned int _h, bool fromMPQ )
  {
    h = _h;

    if( FT_Init_FreeType( &_library ) )
    {
      LogError << "FT_Init_FreeType failed" << std::endl;
      throw std::runtime_error( "FT_Init_FreeType failed" );
    }

    bool failed;
    if( fromMPQ )
    {
      _mpqFile = new MPQFile( fname );
      failed = FT_New_Memory_Face( _library, _mpqFile->get<FT_Byte>( 0 ), _mpqFile->getSize(), 0, &_face );
    }
    else
    {
      failed = FT_New_Face( _library, fname.c_str(), 0, &_face );
    }

    if( failed )
    {
      LogError << "FT_New_Face failed (there is probably a problem with your font file)" << std::endl;
      throw std::runtime_error( "FT_New_Face failed (there is probably a problem with your font file)" );
    }

    // For some twisted reason, Freetype measures font size in terms of 1/64ths of pixels.
    FT_Set_Char_Size( _face, h << 6, h << 6, 72, 72 );
  }

  font_data::~font_data()
  {
    FT_Done_Face( _face );
    FT_Done_FreeType( _library );

    delete _mpqFile;

     typedef std::map<CharacterCode, GlyphData> GlyphListType;

    for( GlyphListType::iterator it( _cachedGlyphs.begin() ), end( _cachedGlyphs.end() ); it != end; ++it )
    {
      delete it->second._callList;
      delete it->second._texture;
    }
    _cachedGlyphs.clear();
  }

  const GlyphData& font_data::getGlyphData( CharacterCode charCode ) const
  {
    if( !_cachedGlyphs.count( charCode ) )
    {
      createGlyph( charCode );
    }
    return _cachedGlyphs[ charCode ];
  }

  void font_data::print( float x, float y, const std::string& text, float colorR, float colorG, float colorB ) const
  {
    float height( h / 0.90f );

    typedef std::vector<std::string> linesType;
    linesType lines;

    boost::split( lines, text, boost::is_any_of( "\n\r" ) );

    glColor3f( colorR, colorG, colorB );

    glPushAttrib( GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT );
    glMatrixMode( GL_MODELVIEW );
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_2D );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    int verticalPosition( y );

    for( linesType::const_iterator line( lines.begin() ), end( lines.end() ); line != end; ++line )
    {
      glPushMatrix();
      glTranslatef( x, verticalPosition, 0.0f );

      const char* lineBegin = line->c_str();
      const char* lineEnd = lineBegin + line->length();

      for( utf8::iterator<const char*> itr( lineBegin, lineBegin, lineEnd ), itrEnd( lineEnd, lineBegin, lineEnd ); itr != itrEnd; ++itr )
      {
        getGlyphData( *itr ).render();
      }

      verticalPosition += height;
      glPopMatrix();
    }

    glPopAttrib();
  }

  void font_data::shprint( float x, float y, const std::string& text, float colorR, float colorG, float colorB ) const
  {
    print( x + 2.0f, y + 2.0f, text, 0.0f, 0.0f, 0.0f );
    print( x, y, text, colorR, colorG, colorB );
  }

  int font_data::width( const std::string& text ) const
  {
    typedef std::vector<std::string> linesType;
    linesType lines;

    boost::split( lines, text, boost::is_any_of( "\n\r" ) );

    int maximumWidth( 0 );

    for( linesType::const_iterator line( lines.begin() ), end( lines.end() ); line != end; ++line )
    {
      int currentWidth( 0 );

      const char* lineBegin = line->c_str();
      const char* lineEnd = lineBegin + line->length();

      for( utf8::iterator<const char*> itr( lineBegin, lineBegin, lineEnd ), itrEnd( lineEnd, lineBegin, lineEnd ); itr != itrEnd; ++itr )
      {
        currentWidth += getGlyphData( *itr ).width();
      }

      maximumWidth = std::max( maximumWidth, currentWidth );
    }

    return maximumWidth;
  }
}
