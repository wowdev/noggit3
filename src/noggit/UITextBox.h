#ifndef __TEXTBOXUI_H
#define __TEXTBOXUI_H

#include <string>

#include <noggit/UIFrame.h>
#include <noggit/UIText.h>

namespace OpenGL { class Texture; };

class UITextBox : public UIFrame
{
public:
  typedef UITextBox* Ptr;
  typedef void ( *TriggerFunction )( UITextBox::Ptr, const std::string& value );

private:
  OpenGL::Texture* _texture;
  OpenGL::Texture* _textureFocused;

  UIText::Ptr _uiText;
  std::string _value;

  bool _focus;

  TriggerFunction _enterFunction;
  TriggerFunction _updateFunction;

public:
  UITextBox( float xPos, float yPos, float w, float h );
  UITextBox( float xPos, float yPos, float w, float h, TriggerFunction enterFunction );
  virtual ~UITextBox();

  void render() const;

  UIFrame::Ptr processLeftClick( float mx, float my );

  void value( const std::string& pText );
  const std::string& value() const;

  void clear();
};
#endif
