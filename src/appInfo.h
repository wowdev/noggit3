#ifndef __APPINFO_H
#define __APPINFO_H

#include <string>

#include "closeWindowUI.h"

class Gui;
class textUI;

class appInfo : public closeWindowUI
{
private:
  Gui* mainGui;
  textUI* theInfos;
  std::string mModelToLoad;

public:
  appInfo( float x, float y, float width, float height, Gui* setGui );
  ~appInfo();
  void setText( const std::string& t );
};
#endif
