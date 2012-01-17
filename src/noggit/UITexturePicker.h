// UITexturePicker.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#ifndef __TEXTUREPICKER_H
#define __TEXTUREPICKER_H

#include <noggit/UICloseWindow.h>

class nameEntry;
class UITexture;

class UITexturePicker : public UICloseWindow
{
public:
  UITexturePicker( float x, float y, float w, float h );

  void getTextures( nameEntry* lSelection );
  void setTexture( size_t id );

private:
  UITexture* _textures[4];
};

#endif
