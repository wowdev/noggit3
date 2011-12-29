#include <noggit/UICursorSwitcher.h>
#include <noggit/UICheckBox.h>
#include <noggit/UISlider.h>
#include <noggit/Environment.h>

#include <noggit/World.h>

UISlider* RedColorSlider;
UISlider* GreenColorSlider;
UISlider* BlueColorSlider;
UISlider* AlphaColorSlider;

UIToggleGroup* Toggle;


void setRed(float f)
{
  Environment::getInstance()->cursorColorR = f;
}

void setGreen(float f)
{
  Environment::getInstance()->cursorColorG = f;
}

void setBlue(float f)
{
  Environment::getInstance()->cursorColorB = f;
}

void setAlpha(float f)
{
  Environment::getInstance()->cursorColorA = f;
}
void UICursorSwitcher::changeCursor(int Type)
{
  Environment::getInstance()->cursorType = Type;
  Toggle->Activate(Type);
}

float Wwidth = 290;
float Wheight = 250;

UICursorSwitcher::UICursorSwitcher()
  : UICloseWindow (0.0f, 0.0f, Wwidth, Wheight, "Cursor options", true)
{
  float leftMargin = 10.0f;
  float SliderWidth = Wwidth - (leftMargin * 2);

  Toggle = new UIToggleGroup( &Environment::getInstance()->cursorType );
  addChild(new UICheckBox(leftMargin, 30.0f, "Cursor - Disk", Toggle, 1));
  addChild(new UICheckBox(leftMargin + 150.0f, 30.0f, "Cursor - Sphere", Toggle, 2));
  addChild(new UICheckBox(leftMargin, 60.0f, "Cursor - Triangle", Toggle, 3));
  addChild(new UICheckBox(leftMargin + 150.0f, 60.0f, "Cursor - None", Toggle, 0));
  Toggle->Activate(Environment::getInstance()->cursorType);

  RedColorSlider = new UISlider(leftMargin, 110.0f, SliderWidth, 1.0f, 0.00001f);
  RedColorSlider->setText("Red Channel: ");
  RedColorSlider->setValue(Environment::getInstance()->cursorColorR);
  RedColorSlider->setFunc(setRed);

  GreenColorSlider = new UISlider(leftMargin, 150.0f, SliderWidth, 1.0f, 0.00001f);
  GreenColorSlider->setText("Green Channel: ");
  GreenColorSlider->setValue(Environment::getInstance()->cursorColorG);
  GreenColorSlider->setFunc(setGreen);

  BlueColorSlider = new UISlider(leftMargin, 190.0f, SliderWidth, 1.0f, 0.00001f);
  BlueColorSlider->setText("Blue Channel: ");
  BlueColorSlider->setValue(Environment::getInstance()->cursorColorB);
  BlueColorSlider->setFunc(setBlue);

  AlphaColorSlider = new UISlider(leftMargin, 230.0f, SliderWidth, 1.0f, 0.00001f);
  AlphaColorSlider->setText("Alpha Channel: ");
  AlphaColorSlider->setValue(Environment::getInstance()->cursorColorA);
  AlphaColorSlider->setFunc(setAlpha);

  addChild(RedColorSlider);
  addChild(GreenColorSlider);
  addChild(BlueColorSlider);
  addChild(AlphaColorSlider);
}
