#ifndef __APPINFO_H
#define __APPINFO_H

#include <string>

#include "window.h"

class Gui;
class textUI;

class appInfo:public window
{
private:
	Gui *mainGui;
	textUI *theInfos;
  std::string mModelToLoad;

public:
	appInfo(float x, float y, float width, float height, Gui *setGui);
  ~appInfo();
	void setText( const std::string& t );
};
#endif
