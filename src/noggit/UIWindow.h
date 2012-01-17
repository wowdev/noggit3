// UIWindow.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

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
