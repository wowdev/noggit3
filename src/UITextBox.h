#ifndef __TEXTBOXUI_H
#define __TEXTBOXUI_H

#include <string>

#include "UIFrame.h"

class UIText;
class UITexture;
class SDL_KeyboardEvent;
namespace OpenGL { class Texture; };

//! \todo  Combine and get it working.
class UITextBox : public UIFrame
{
protected:
  char text[256];
  int length;
  UITexture* background;
  UIText* theText;
  
public:
  UITextBox( float xpos, float ypos, float w );
  UIFrame* processLeftClick( float mx, float my );
  bool processKey( char key, bool shift, bool alt, bool ctrl );
};

class UITextBox2 : public UIFrame
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
  UITextBox2(float xPos,float yPos,float w, float h,const std::string& tex, const std::string& texd);
  ~UITextBox2();
  void render() const;

  void setValue( const std::string& pText );
  std::string  getValue();

  bool KeyBoardEvent( SDL_KeyboardEvent *e );
  
  UIFrame *processLeftClick( float mx, float my );
  void processUnclick() { }
};
#endif
