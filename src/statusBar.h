#ifndef __STATUSBAR_H
#define __STATUSBAR_H

class statusBar;

#include "window.h"
#include "textUI.h"

class statusBar : public window
{
private:
	textUI *leftInfo;
	textUI *rightInfo;

public:
	statusBar( float x, float y, float width, float height );
	void render( );	
	void resize( );
	void setLeftInfo( const std::string& pText );
	void setRightInfo( const std::string& pText );
};
#endif
