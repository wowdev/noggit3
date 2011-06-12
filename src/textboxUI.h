#ifndef __TEXTBOXUI_H
#define __TEXTBOXUI_H

#include "frame.h"
#include <string>

class textUI;
class textureUI;
class SDL_KeyboardEvent;
namespace OpenGL { class Texture; };

//! \todo  Combine and get it working.
class textboxUI:public frame
{
protected:
  char    text[256];
  int      length;
  textureUI  *background;
  textUI    *theText;
public:
  textboxUI(float xpos, float ypos, float w);
  frame *processLeftClick(float mx,float my);
  bool processKey(char key, bool shift, bool alt, bool ctrl);
  //void render();
};

class TextBox : public frame
{
private:
  OpenGL::Texture* texture;
  OpenGL::Texture* textureDown;

  bool  mFocus;
  textUI  *mText;
  std::string mValue;
public:
  TextBox(float xPos,float yPos,float w, float h,const std::string& tex, const std::string& texd);
  void render() const;

  void setValue( const std::string& pText );
  std::string  getValue();

  bool KeyBoardEvent( SDL_KeyboardEvent *e );
  
  frame *processLeftClick( float mx, float my );
  void processUnclick() { }
};
#endif
