// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CursorSwitcher.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Slider.h>
#include <noggit/Environment.h>

#include <noggit/World.h>

UICursorSwitcher::UICursorSwitcher(math::vector_4d& color, int& cursor_type)
  : UICloseWindow ( (float)video.xres() / 2.0f - 290.f / 2.0f
                  , (float)video.yres() / 2.0f - 250.f / 2.0f
                  , 290.f
                  , 250.f
                  , "Cursor options"
                  , true
                  )
{
  float leftMargin = 10.0f;
  float SliderWidth = width() - (leftMargin * 2);

  auto Toggle = new UIToggleGroup(&cursor_type);
  addChild(new UICheckBox(leftMargin, 30.0f, "Cursor - Disk", Toggle, 1));
  addChild(new UICheckBox(leftMargin + 150.0f, 30.0f, "Cursor - Sphere", Toggle, 2));
  addChild(new UICheckBox(leftMargin, 60.0f, "Cursor - Triangle", Toggle, 3));
  addChild(new UICheckBox(leftMargin + 150.0f, 60.0f, "Cursor - None", Toggle, 0));
  Toggle->Activate(cursor_type);

  auto RedColorSlider = new UISlider(leftMargin, 110.0f, SliderWidth, 1.0f, 0.0f);
  RedColorSlider->setText("Red Channel: ");
  RedColorSlider->setValue(color.x);
  RedColorSlider->setFunc([&](float f) { color.x = f; });

  auto GreenColorSlider = new UISlider(leftMargin, 150.0f, SliderWidth, 1.0f, 0.0f);
  GreenColorSlider->setText("Green Channel: ");
  GreenColorSlider->setValue(color.y);
  GreenColorSlider->setFunc([&](float f) { color.y = f; });

  auto BlueColorSlider = new UISlider(leftMargin, 190.0f, SliderWidth, 1.0f, 0.0f);
  BlueColorSlider->setText("Blue Channel: ");
  BlueColorSlider->setValue(color.z);
  BlueColorSlider->setFunc([&](float f) { color.z = f; });

  auto AlphaColorSlider = new UISlider(leftMargin, 230.0f, SliderWidth, 1.0f, 0.0f);
  AlphaColorSlider->setText("Alpha Channel: ");
  AlphaColorSlider->setValue(color.w);
  AlphaColorSlider->setFunc([&](float f) { color.w = f; });

  addChild(RedColorSlider);
  addChild(GreenColorSlider);
  addChild(BlueColorSlider);
  addChild(AlphaColorSlider);
}
