#ifndef FREE_TYPE_H
#define FREE_TYPE_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H 

#include <GL/glew.h>

#include <vector>
#include <string>
#include "mpq.h"
//Using the STL exception library increases the
//chances that someone else using our code will corretly
//catch any exceptions that we throw.
#include <stdexcept>


//MSVC will spit out all sorts of useless warnings if
//you create vectors of strings, this pragma gets rid of them.
#ifdef WIN32
#pragma warning(disable: 4786)
#endif

///Wrap everything in a namespace, that we can use common
///function names like "print" without worrying about
///overlapping with anyone else's code.
namespace freetype {

class font_data {
public:
  float h;      ///< Holds the height of the font.
  GLuint * textures;  ///< Holds the texture id's 
  GLuint list_base;  ///< Holds the first display list id
  int    charWidths[128];

  font_data() {}
  font_data( const std::string& fname, unsigned int h, bool fromMPQ );

  ~font_data();
};

// The flagship function of the library - this thing will print out text at window coordinates x,y, using the font ft_font. The current modelview matrix will also be applied to the text. 
void print(const font_data &ft_font, float x, float y, const std::string& text, float colorR = 1.0f, float colorG = 1.0f, float colorB = 1.0f) ;
void shprint(const font_data &ft_font, float x, float y, const std::string& text, float colorR = 1.0f, float colorG = 1.0f, float colorB = 1.0f );
int width(const font_data &ft_font, const std::string& text) ;

}

#endif
