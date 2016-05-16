// UITexturePicker.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#pragma once

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
