#ifndef UIALPHAMAP_H
#define UIALPHAMAP_H

#include "UICloseWindow.h"

class UIAlphamap : public UICloseWindow
{
public:
  UIAlphamap( float x, float y);

  void render() const;

};
#endif
