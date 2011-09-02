#ifndef __TEXTBOXUI_H
#define __TEXTBOXUI_H

#include <string>

#include "UIFrame.h"

class UIText;
class UITexture;
struct SDL_KeyboardEvent;
namespace OpenGL { class Texture; };

class UITextBox : public UIFrame
{
private:
  OpenGL::Texture* texture;
  OpenGL::Texture* textureDown;
  std::string _textureFilename;
  std::string _textureDownFilename;

  bool  mFocus;
  UIText  *mText;
  std::string mValue;
public:
  UITextBox(float xPos,float yPos,float w, float h,const std::string& tex, const std::string& texd);
  ~UITextBox();
  void render() const;

  void setValue( const std::string& pText );
  void Clear();
  std::string  getValue();

  bool KeyBoardEvent( SDL_KeyboardEvent *e );
  
  UIFrame *processLeftClick( float mx, float my );
  void processUnclick() { }
};
#endif
