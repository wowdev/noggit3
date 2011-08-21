#ifndef __UIBUTTON_H
#define __UIBUTTON_H

#include <string>

#include "UIFrame.h"

namespace OpenGL { class Texture; };
class UIText;
namespace freetype { class font_data; };
  
class UIButton : public UIFrame
{
protected:
  OpenGL::Texture* texture;
  OpenGL::Texture* textureDown;
  std::string _textureFilename;
  std::string _textureDownFilename;
  
  void ( *clickFunc )( UIFrame *, int );
  int id;

  bool clicked;
  UIText* text;

public:
  UIButton( float x, float y, float width, float height, const std::string& pTexNormal, const std::string& pTexDown );
  UIButton( float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown );
  UIButton( float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown, void (*pFunc)( UIFrame *, int ), int pFuncParam );
  ~UIButton();
  
  void render() const;

  void setLeft();
  void setText( const std::string& pText );

  UIFrame* processLeftClick( float mx, float my );
  void setClickFunc( void (*f)( UIFrame *, int ), int num );
  void processUnclick();
};
#endif
