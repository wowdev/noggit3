#ifndef FREE_TYPE_H
#define FREE_TYPE_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <string>

#include <noggit/Video.h>
#include <noggit/MPQ.h>

namespace freetype
{
  struct GlyphData
  {
    OpenGL::CallList* _callList;
    OpenGL::Texture* _texture;
    int _width;

    inline void render() const
    {
      _callList->render();
    }
    inline const int& width() const
    {
      return _width;
    }
  };

  class font_data
  {
  public:
    unsigned int h;

    font_data()
    : _mpqFile( NULL )
    {
    }
    ~font_data();

    void init( const std::string& fname, unsigned int h, bool fromMPQ );
    int width( const std::string& text ) const;
    void print( float x, float y, const std::string& text, float colorR = 1.0f, float colorG = 1.0f, float colorB = 1.0f ) const;
    void shprint( float x, float y, const std::string& text, float colorR = 1.0f, float colorG = 1.0f, float colorB = 1.0f ) const;

  private:
    typedef FT_ULong CharacterCode;
    //! \todo Use some hash map.
    typedef std::map<CharacterCode, GlyphData> GlyphListType;

    mutable GlyphListType _cachedGlyphs;

    FT_Face _face;
    FT_Library _library;

    MPQFile* _mpqFile;

    void createGlyph( CharacterCode charCode ) const;
    const GlyphData& getGlyphData( CharacterCode charCode ) const;
  };
}

#endif
