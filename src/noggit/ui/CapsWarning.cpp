// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CapsWarning.h>

#include <noggit/application.h> // fonts
#include <noggit/ui/Text.h>
#include <noggit/ui/Texture.h>
#include <noggit/Video.h> // video

UICapsWarning::UICapsWarning()
	: UIWindow((float)video.xres() / 2.0f - (float)winWidth / 2.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f - (video.yres() / 4), (float)winWidth, (float)winHeight)
{
	addChild(new UITexture(10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Sigil_Thorim.blp"));
	addChild(new UIText(95.0f, 20.0f, "Caps lock in on!", app.getArial14(), eJustifyLeft));
	addChild(new UIText(95.0f, 40.0f, "Turn it off to work with noggit.", app.getArial14(), eJustifyLeft));
}

void UICapsWarning::resize()
{
	x(std::max((video.xres() / 2.0f) - (winWidth / 2.0f), 0.0f));
	y(std::max((video.yres() / 2.0f) - (winHeight / 2.0f), 0.0f) - (video.yres() / 4));
}
