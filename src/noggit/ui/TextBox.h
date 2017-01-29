// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/TextureManager.h>
#include <noggit/ui/Frame.h>
#include <noggit/ui/Text.h>

#include <functional>

namespace OpenGL { class Texture; };

class UITextBox : public UIFrame
{
public:
  typedef UITextBox* Ptr;
  typedef std::function<void(UITextBox::Ptr, const std::string&)> TriggerFunction;

private:
  scoped_blp_texture_reference _texture;
  scoped_blp_texture_reference _textureFocused;

  UIText::Ptr _uiText;
  std::string _value;

  bool _focus;

  TriggerFunction _enterFunction;
  TriggerFunction _updateFunction;

public:
  UITextBox(float xPos, float yPos, float w, float h);
  UITextBox(float xPos, float yPos, float w, float h, TriggerFunction enterFunction);
  UITextBox(float xPos, float yPos, float w, float h, const freetype::font_data& pFont);
  UITextBox(float xPos, float yPos, float w, float h, const freetype::font_data& pFont, TriggerFunction enterFunction);

  void render() const;

  UIFrame::Ptr processLeftClick(float mx, float my);

  bool key_down (SDLKey sym, Uint16 unicode);

  void value(const std::string& pText);
  const std::string& value() const;
  bool focus() const { return _focus; }

  void clear();
};
