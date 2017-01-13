// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include "UICloseWindow.h"
#include "UIToggleGroup.h"

class UICursorSwitcher : public UICloseWindow
{
public:
	UICursorSwitcher();

	void changeCursor(int Type);
};
