#ifndef __TEXTUREUI_H
#define __TEXTUREUI_H

#include <string>

#include <noggit/UIFrame.h>

namespace OpenGL
{
  class Texture;
}

class UITexture : public UIFrame
{
public:
  typedef UITexture* Ptr;

protected:
  OpenGL::Texture* texture;
  std::string _textureFilename;

  bool highlight;
  void (*clickFunc)(UIFrame *,int);
  int id;

public:
  UITexture( float x, float y, float width, float height, const std::string& tex );
  ~UITexture();

  void setTexture( const std::string& tex );
  void setTexture( OpenGL::Texture* tex );
  void render() const;

  UIFrame *processLeftClick( float mx, float my );
  void setClickFunc( void (*f)( UIFrame *,int ), int num );
  void setHighlight( bool h )
  {
    highlight = h;
  }
  OpenGL::Texture* getTexture( );
};

#endif
