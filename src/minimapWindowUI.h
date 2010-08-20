#ifndef __MINIMAPWINDOWUI_H
#define __MINIMAPWINDOWUI_H

#include "closeWindowUI.h"

class minimapWindowUI : public closeWindowUI
{
public:
	minimapWindowUI( float x, float y, float w, float h, const std::string pTitle );
};

#endif