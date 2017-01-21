// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/WaterSaveWarning.h>

#include <noggit/application.h> // fonts
#include <noggit/ui/Text.h>
#include <noggit/ui/Texture.h>
#include <noggit/Video.h> // video

UIWaterSaveWarning::UIWaterSaveWarning()
  : UIWindow(video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f - (video.yres() / 2.5f), winWidth, winHeight)
{
  addChild(new UITexture(10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\Ability_Creature_Poison_06.blp"));
  addChild(new UIText(95.0f, 20.0f, "Old style water! Noggit will not", app.getArial14(), eJustifyLeft));
  addChild(new UIText(95.0f, 40.0f, "save some water on this ADT!", app.getArial14(), eJustifyLeft));
}

void UIWaterSaveWarning::resize()
{
  x(std::max((video.xres() / 2.0f) - (winWidth / 2.0f), 0.0f));
  y(std::max((video.yres() / 2.0f) - (winHeight / 2.0f), 0.0f) - (video.yres() / 4));
}
