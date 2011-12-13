#include "UICursorSwitcher.h"
#include "UICheckBox.h"

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

float Wwidth = 765;
float Wheight = 400;

//UICursorSwitcher::UICursorSwitcher(float x, float y, float w, float h) : UICloseWindow(x, y, w, h, "Select the cursor you want to switch.", true)
UICursorSwitcher::UICursorSwitcher() : UICloseWindow((float)video.xres() / 2.0f - Wwidth / 2.0f, (float)video.yres() / 2.0f - Wheight / 2.0f, Wwidth, Wheight, "Select the cursror you want to switch.", true)
{
	addChild(new UICheckBox(30.0f, 30.0f, "Cursor - Disk", SwitchDisk, 0));
	addChild(new UICheckBox(90.0f, 30.0f, "Cursor - Sphere", SwitchSphere, 0));
}