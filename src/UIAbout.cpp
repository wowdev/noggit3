#include "UIAbout.h"

#include <algorithm>

#include "Noggit.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UITexture.h"
#include "Video.h" // video

#include "Log.h"

UIAbout::UIAbout()
	: UICloseWindow((float)video.xres() / 2.0f - (float)winWidth / 2.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f, (float)winWidth, (float)winHeight, "")
{
	addChild(new UITexture(20.0f, 20.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Potion_83.blp"));
	addChild(new UIText(73.0f, 24.0f, "Noggit Studio", app.getSkurri32(), eJustifyLeft));
	addChild(new UIText(155.0f, 57.0f, "a wow map editor for 3.3.5a", app.getFritz16(), eJustifyLeft));
	addChild(new UIText(20.0f, 95.0f, "Ufoz [...],   Cryect,   Beket,   Schlumpf,   Tigurius", app.getFritz16(), eJustifyLeft));
	addChild(new UIText(20.0f, 115.0f, "Steff,  Garthog,  Glararan,  Cromon,  Hanfer", app.getFritz16(), eJustifyLeft));
	addChild(new UIText(20.0f, 135.0f, "Skarn,  AxelSheva,  Valium,  Kaev,  Adspartan", app.getFritz16(), eJustifyLeft));
	addChild(new UIText(20.0f, 165.0f, "World of Warcraft is (C) Blizzard Entertainment", app.getFritz16(), eJustifyLeft));
	addChild(new UIText(20.0f, 190.0f, STRPRODUCTVER, app.getFritz16(), eJustifyLeft));
	addChild(new UIText(375.0f, 190.0f, __DATE__ ", " __TIME__, app.getFritz16(), eJustifyRight));
}

void UIAbout::resize()
{
	x(std::max((video.xres() / 2.0f) - (winWidth / 2.0f), 0.0f));
	y(std::max((video.yres() / 2.0f) - (winHeight / 2.0f), 0.0f));
}
