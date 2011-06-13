#ifndef __APPINFO_H
#define __APPINFO_H

#include <string>

#include "UICloseWindow.h"

class UIMapViewGUI;
class UIText;

class UIAppInfo : public UICloseWindow
{
private:
  UIMapViewGUI* mainGui;
  UIText* theInfos;
  std::string mModelToLoad;

public:
  UIAppInfo( float x, float y, float width, float height, UIMapViewGUI* setGui );
  ~UIAppInfo();
  void setText( const std::string& t );
};
#endif
