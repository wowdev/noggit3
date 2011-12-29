#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>

#include <noggit/FreeType.h> // fonts.
#include <noggit/UIFrame.h>

namespace noggit
{
  class blp_texture;
}

class UIWindow : public UIFrame
{
public:
  typedef UIWindow* Ptr;

protected:
  noggit::blp_texture* texture;
  std::string _textureFilename;

public:
  UIWindow( float xPos, float yPos, float w, float h );
  UIWindow( float xPos, float yPos, float w, float h, const std::string& pTexture );
  ~UIWindow();
  UIFrame::Ptr processLeftClick( float mx, float my );
  void render() const;
};

#endif
