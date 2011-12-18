#include "UICursorSwitcher.h"
#include "UICheckBox.h"
#include "UISlider.h"
#include "UIToggleGroup.h"

#include "World.h"

UISlider* RedColorSlider;
UISlider* GreenColorSlider;
UISlider* BlueColorSlider;
UISlider* AlphaColorSlider;

UIToggleGroup* Toggle;

float SliderWidth = 200.0f;

extern bool cursorDisk;
extern bool cursorSphere;
extern int cursorType;

void UICursorSwitcher::changeCursor(int Type)
{
	if(Type == 0)
	{
		cursorSphere = false;
		cursorDisk = true;
	}
	else if(Type == 1)
	{
		cursorDisk = false;
		cursorSphere = true;
	}
}

float Wwidth = 480;
float Wheight = 320;

UICursorSwitcher::UICursorSwitcher() : UICloseWindow((float)video.xres() / 2.0f - Wwidth / 2.0f, (float)video.yres() / 2.0f - Wheight / 2.0f, Wwidth, Wheight, "Select the cursror you want to switch.", true)
{
	Toggle = new UIToggleGroup(&cursorType);
	addChild(new UICheckBox(30.0f, 30.0f, "Cursor - Disk", Toggle, 0));
	addChild(new UICheckBox(150.0f, 30.0f, "Cursor - Sphere", Toggle, 1));
	Toggle->Activate(0);

	RedColorSlider = new UISlider(30.0f, 80.0f, SliderWidth, 1.0f, 0.00001f);
	RedColorSlider->setText("Red Channel: ");
	RedColorSlider->setValue(RedColor());

	GreenColorSlider = new UISlider(30.0f, 120.0f, SliderWidth, 1.0f, 0.00001f);
	GreenColorSlider->setText("Green Channel: ");
	GreenColorSlider->setValue(GreenColor());

	BlueColorSlider = new UISlider(30.0f, 160.0f, SliderWidth, 1.0f, 0.00001f);
	BlueColorSlider->setText("Blue Channel: ");
	BlueColorSlider->setValue(BlueColor());

	AlphaColorSlider = new UISlider(30.0f, 200.0f, SliderWidth, 1.0f, 0.00001f);
	AlphaColorSlider->setText("Alpha Channel: ");
	AlphaColorSlider->setValue(AlphaColor());

	addChild(RedColorSlider);
	addChild(GreenColorSlider);
	addChild(BlueColorSlider);
	addChild(AlphaColorSlider);
}