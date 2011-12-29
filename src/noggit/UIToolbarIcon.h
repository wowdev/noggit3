#ifndef __ICON_H
#define __ICON_H

#include <string>

#include <noggit/UIEventClasses.h>
#include <noggit/UIFrame.h>

namespace noggit
{
  class blp_texture;
}

class UIToolbarIcon : public UIFrame, public UIEventSender
{
public:
  UIEventEventHandlerDefinition(int);
protected:
  noggit::blp_texture* texture;
  noggit::blp_texture* textureSelected;

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
