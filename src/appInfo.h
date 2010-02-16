#ifndef __APPINFO_H
#define __APPINFO_H

class appInfo;

#include "Gui.h"
#include "textUI.h"
#include "video.h"
#include "window.h"

class appInfo:public window
{
private:
	Gui *mainGui;
	textUI *theInfos;

public:
	appInfo(float x, float y, float width, float height, Gui *setGui);
	void setText( std::string t );
};
#endif