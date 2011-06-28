#ifndef __TEXTUREPICKER_H
#define __TEXTUREPICKER_H

#include "Selection.h"
#include "UIButton.h"
#include "UITexture.h"
#include "UIWindow.h"

class UIMapViewGUI;

class UITexturePicker : public UIWindow
{
public:
  UITexturePicker(int xPos,int yPos, int w, int h, UIMapViewGUI *setGui);
  void getTextures(nameEntry *lSelection);
  void setTexture(int id);
  UIMapViewGUI *mainGUI;
  
private:
  UITexture* tex1;
  UITexture* tex2;
  UITexture* tex3;
  UITexture* tex4;
};

#endif