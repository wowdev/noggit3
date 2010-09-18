#ifndef __DETAILINFOS_H
#define __DETAILINFOS_H

class detailInfos;

#include "Gui.h"
#include "textUI.h"
#include "window.h"

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
