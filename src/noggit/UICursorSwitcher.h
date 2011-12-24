#ifndef CURSORSWITCHER_H
#define CURSORSWITCHER_H

#include <noggit/UICloseWindow.h>
#include <noggit/UIToggleGroup.h>

class UICursorSwitcher : public UICloseWindow
{
public:
  UICursorSwitcher();

  void changeCursor(int Type);
};

#endif
