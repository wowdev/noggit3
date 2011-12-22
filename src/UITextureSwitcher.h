#ifndef __TEXTURESWITCHER_H
#define __TEXTURESWITCHER_H

#include "UICloseWindow.h"

class nameEntry;
class UITexture;

class UITextureSwitcher : public UICloseWindow
{
public:
  UITextureSwitcher(  int x, int y );

  void getTextures( nameEntry* lSelection );
  void setTexture( size_t id );
  void setPosition( int x, int y );

private:
  UITexture* _textures;
  float xPos, zPos;
};

#endif
