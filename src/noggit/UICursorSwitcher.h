// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UICloseWindow.h>
#include <noggit/UIToggleGroup.h>

class UICursorSwitcher : public UICloseWindow
{
public:
	UICursorSwitcher();

	void changeCursor(int Type);
};
