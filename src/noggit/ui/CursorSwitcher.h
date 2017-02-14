// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>
#include <noggit/ui/ToggleGroup.h>

class UICursorSwitcher : public UICloseWindow
{
public:
  UICursorSwitcher(math::vector_4d& color, int& cursor_type);
};
