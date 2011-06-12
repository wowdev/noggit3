#ifndef __STATUSBAR_H
#define __STATUSBAR_H

#include <string>

#include "window.h"
#include "FreeType.h" // fonts.

class textUI;

class statusBar : public window
{
private:
	textUI *leftInfo;
	textUI *rightInfo;

public:
	statusBar( float x, float y, float width, float height );
	void render() const;	
	void resize();
	void setLeftInfo( const std::string& pText );
	void setRightInfo( const std::string& pText );
};
#endif
