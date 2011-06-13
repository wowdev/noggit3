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

#include "MPQ.h"

#include <stdexcept>

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

void print(const font_data &ft_font, float x, float y, const std::string& text, float colorR = 1.0f, float colorG = 1.0f, float colorB = 1.0f) ;
void shprint(const font_data &ft_font, float x, float y, const std::string& text, float colorR = 1.0f, float colorG = 1.0f, float colorB = 1.0f );
int width(const font_data &ft_font, const std::string& text) ;

}

#endif
