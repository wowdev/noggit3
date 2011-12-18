#ifndef CURSORSWITCHER_H
#define CURSORSWITCHER_H

#include "UICloseWindow.h"

class UICursorSwitcher : public UICloseWindow
{
public:
	UICursorSwitcher();

	void changeCursor(int Type);
};

#endif