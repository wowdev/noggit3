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

float Wwidth = 480;
float Wheight = 320;

UICursorSwitcher::UICursorSwitcher() : UICloseWindow((float)video.xres() / 2.0f - Wwidth / 2.0f, (float)video.yres() / 2.0f - Wheight / 2.0f, Wwidth, Wheight, "Select the cursror you want to switch.", true)
{
	addChild(new UICheckBox(30.0f, 30.0f, "Cursor - Disk", SwitchDisk, 0));
	addChild(new UICheckBox(150.0f, 30.0f, "Cursor - Sphere", SwitchSphere, 0));
}