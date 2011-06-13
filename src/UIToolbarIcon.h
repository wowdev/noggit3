#ifndef __ICON_H
#define __ICON_H

#include <string>

#include "UIEventClasses.h"
#include "UIFrame.h"

namespace OpenGL { class Texture; };

class UIToolbarIcon : public UIFrame, public UIEventSender
{
public:
  UIEventEventHandlerDefinition(int);
protected:
  OpenGL::Texture* texture;
  OpenGL::Texture* textureSelected;
  int iconId;

public:
  UIToolbarIcon( float x, float y, float width, float height, const std::string& tex, const std::string& texd, const int& id, UIEventClassConstructorArguments );

  void render() const;
  UIFrame *processLeftClick(float mx,float my);
  
  bool selected;
};
#endif
