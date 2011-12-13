#include "UICursorSwitcher.h"
#include "UICheckBox.h"

class UICloseWindow;
UICloseWindow* cursorSwitch;

extern bool cursorDisk;
extern bool cursorSphere;

void SwitchSphere(bool /*f*/, int /*id*/)
{
	cursorDisk = false;
	cursorSphere = true;
}

void SwitchDisk(bool /*f*/, int /*id*/)
{
	cursorSphere = false;
	cursorDisk = true;
}

UICursorSwitcher::UICursorSwitcher(float x, float y, float w, float h) : UICloseWindow(x, y, w, h, "Select the cursor you want to switch.", true)
{
	cursorSwitch->addChild(new UICheckBox(30.0f, 30.0f, "Cursor - Disk", SwitchDisk, 0));
	cursorSwitch->addChild(new UICheckBox(90.0f, 30.0f, "Cursor - Sphere", SwitchSphere, 0));
}