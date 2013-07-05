#ifndef UIALPHAMAP_H
#define UIALPHAMAP_H

#include "UICloseWindow.h"

class UIAlphamap : public UICloseWindow
{
public:
  UIAlphamap( float x, float y);

  void render() const;

private:
  void drawQuad(size_t i, size_t j) const;

};
#endif
