#ifndef __TEXTURESWITCHER_H
#define __TEXTURESWITCHER_H

#include "UICloseWindow.h"

class nameEntry;
class UITexture;

class UITextureSwitcher : public UICloseWindow
{
public:
  UITextureSwitcher( float x, float y, float w, float h );

  void getTextures( nameEntry* lSelection );
  void setTexture( size_t id );

private:
  UITexture* _textures[4];
  float xPos, zPos;
};

#endif
