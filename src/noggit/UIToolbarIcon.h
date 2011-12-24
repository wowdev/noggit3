#ifndef __ICON_H
#define __ICON_H

#include <string>

#include <noggit/UIEventClasses.h>
#include <noggit/UIFrame.h>

namespace OpenGL { class Texture; };

class UIToolbarIcon : public UIFrame, public UIEventSender
{
public:
  UIEventEventHandlerDefinition(int);
protected:
  OpenGL::Texture* texture;
  OpenGL::Texture* textureSelected;

  std::string _textureFilename;
  std::string _textureSelectedFilename;

  int iconId;

public:
  UIToolbarIcon( float x, float y, const std::string& tex, const std::string& texd, const int& id, UIEventClassConstructorArguments );
  ~UIToolbarIcon();

  void render() const;
  UIFrame *processLeftClick(float mx,float my);

  bool selected;
};
#endif
