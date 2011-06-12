#ifndef __TEXTUI_H
#define __TEXTUI_H

#include <string>

#include "frame.h"

namespace freetype { class font_data; }

enum eJustify
{
  eJustifyLeft = 0,
  eJustifyCenter = 1,
  eJustifyRight = 2
};

class textUI : public frame
{
private:
  int twidth;

protected:
  freetype::font_data  *font;
  std::string mText;
  int justify;
  bool background;
  float bgColor[4];

public:
  textUI( float pX, float pY, const std::string& pText, freetype::font_data *pFont, int pJustify );
  textUI( float pX, float pY, freetype::font_data *pFont, int pJustify );
  void setText( const std::string& pText );
  void setJustify( int j );
  void setFont( freetype::font_data *font );
  void setBackground( float r, float g, float b, float a );
  void render() const;
  const std::string& getText() const
  {
    return mText;
  }
};
#endif
