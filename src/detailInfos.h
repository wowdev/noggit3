#ifndef __DETAILINFOS_H
#define __DETAILINFOS_H

#include <string>

#include "window.h"

class detailInfos;
class Gui;
class textUI;

class detailInfos : public window
{
private:
	Gui *mainGui;
	textUI *theInfos;

public:
	detailInfos( float x, float y, float width, float height, Gui *setGui );
	void setText( const std::string& t );
};
#endif
