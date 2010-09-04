#ifndef __WIN_CREDITS_H
#define __WIN_CREDITS_H

#include "closeWindowUI.h"

class winCredits : public closeWindowUI
{
private:
	static const int winWidth  = 380;
	static const int winHeight = 200;
public:
	winCredits();
	void resize();
};

#endif
