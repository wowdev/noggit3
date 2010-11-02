#ifndef __CLOSEWINDOWUI_H
#define __CLOSEWINDOWUI_H

#include "window.h"
#include "FreeType.h" // fonts.

class closeWindowUI : public window
{
public:
	closeWindowUI( float x, float y, float w, float h, const std::string& pTitle, bool pMoveable = false );
};

#endif
