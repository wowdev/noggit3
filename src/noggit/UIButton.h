#ifndef __UIBUTTON_H
#define __UIBUTTON_H

#include <string>

#include <noggit/UIFrame.h>
#include <noggit/UIText.h>

namespace noggit
{
  class blp_texture;
}
namespace freetype
{
  class font_data;
}

class UIButton : public UIFrame
{
public:
  typedef UIButton* Ptr;

protected:
  noggit::blp_texture* texture;
  noggit::blp_texture* textureDown;
  std::string _textureFilename;
  std::string _textureDownFilename;

  typedef void ( *ClickFunction )( UIFrame::Ptr, int );
  ClickFunction clickFunc;
  int id;

  bool clicked;
  UIText::Ptr text;

public:
  explicit UIButton( float x, float y, float width, float height, const std::string& pTexNormal, const std::string& pTexDown );
  explicit UIButton( float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown );
  explicit UIButton( float x, float y, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown );
  explicit UIButton( float x, float y, float width, float height, const std::string& pText, const std::string& pTexNormal, const std::string& pTexDown, ClickFunction pFunc, int pFuncParam );
  ~UIButton();

  void render() const;

  void setLeft();
  void setText( const std::string& pText );

  UIFrame::Ptr processLeftClick( float mx, float my );
  void setClickFunc( ClickFunction f, int num );
  void processUnclick();
};
#endif
