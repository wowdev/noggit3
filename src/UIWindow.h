#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>

#include "FreeType.h" // fonts.
#include "UIFrame.h"

namespace OpenGL { class Texture; };

class UIWindow : public UIFrame
{
protected:
  OpenGL::Texture* texture;

public:
  UIWindow( float xPos, float yPos, float w, float h );
  UIWindow( float xPos, float yPos, float w, float h, const std::string& pTexture );
  UIFrame* processLeftClick( float mx, float my );
  void render() const;
};

#endif
