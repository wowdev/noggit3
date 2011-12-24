#ifndef __TEXTUI_H
#define __TEXTUI_H

#include <string>

#include <noggit/UIFrame.h>

namespace freetype { class font_data; }

enum eJustify
{
  eJustifyLeft = 0,
  eJustifyCenter = 1,
  eJustifyRight = 2
};

class UIText : public UIFrame
{
public:
  typedef UIText* Ptr;

protected:
  const freetype::font_data& font;
  std::string mText;
  int justify;
  bool background;
  float bgColor[4];

public:
  UIText( float pX, float pY, const std::string& pText, const freetype::font_data& pFont, eJustify pJustify );
  UIText( float pX, float pY, const freetype::font_data& pFont, eJustify pJustify );
  void setText( const std::string& pText );
  void setJustify( eJustify j );
  void setBackground( float r, float g, float b, float a );
  void render() const;
  const std::string& getText() const
  {
    return mText;
  }
};
#endif
