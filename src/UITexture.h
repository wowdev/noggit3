#ifndef __TEXTUREUI_H
#define __TEXTUREUI_H

#include <string>

#include "TextureManager.h"
#include "UIFrame.h"

class UITexture : public UIFrame
{
protected:
  OpenGL::Texture* texture;
  bool highlight;
  void (*clickFunc)(UIFrame *,int);
  int id;

public:
  //UITexture( float x, float y, float width, float height, GLuint tex );
  UITexture( float x, float y, float width, float height, const std::string& tex );
  void setTexture( GLuint tex );
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
